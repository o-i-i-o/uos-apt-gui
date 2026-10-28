#!/bin/bash
# 阶段 5 安全回归：TOCTOU 暂存、参数/暂存校验、Busy 与 fail-closed。
# 依赖：dbus-daemon、python3-gi（缺失时以 77 退出表示跳过）。
set -u

REPO_DIR=$(cd "$(dirname "$0")/.." && pwd)
BUILD_DIR=${BUILD_DIR:-$REPO_DIR/build}
HELPER=$BUILD_DIR/apt-helperd
CLIENT=$REPO_DIR/tests/security_client.py
FAKE_POLKIT=$REPO_DIR/tests/fake_polkit.py
ALLOW_ALL="com.debmanager.helper.install,com.debmanager.helper.remove,com.debmanager.helper.update-cache,com.debmanager.helper.upgrade"

if ! python3 -c "import gi" >/dev/null 2>&1; then
    echo "SKIP: 需要 python3-gi"
    exit 77
fi
if ! command -v dbus-daemon >/dev/null 2>&1; then
    echo "SKIP: 需要 dbus-daemon"
    exit 77
fi
if [ ! -x "$HELPER" ]; then
    echo "未找到 $HELPER，请先构建"
    exit 1
fi

WORK=$(mktemp -d)
STAGING=$WORK/staging
BAD_STAGING=$WORK/bad-staging
mkdir -m 0700 "$STAGING"
mkdir -m 0777 "$BAD_STAGING"
chmod 0777 "$BAD_STAGING"

ADDR=""
PIDS=()
CHECKS=0
FAILURES=0

stop_stack() {
    local pid
    for pid in "${PIDS[@]}"; do
        kill "$pid" 2>/dev/null
    done
    wait 2>/dev/null
    PIDS=()
}

cleanup() {
    stop_stack
    rm -rf "$WORK"
}
trap cleanup EXIT

start_stack() {
    local allow=$1 delay=$2 staging=$3
    stop_stack

    local out
    out=$(dbus-daemon --session --fork --print-address=1 --print-pid=1) || return 1
    ADDR=$(printf '%s\n' "$out" | head -n1)
    PIDS+=("$(printf '%s\n' "$out" | tail -n1)")

    FAKE_POLKIT_ALLOW="$allow" FAKE_POLKIT_DELAY_MS="$delay" \
        python3 "$FAKE_POLKIT" "$ADDR" >"$WORK/polkit.log" 2>&1 &
    PIDS+=($!)

    DBUS_SYSTEM_BUS_ADDRESS="$ADDR" "$HELPER" --dry-run --staging-dir "$staging" \
        >"$WORK/helper.log" 2>&1 &
    PIDS+=($!)

    local i
    for i in $(seq 1 100); do
        if timeout 2 gdbus introspect --address "$ADDR" --dest com.debmanager.Helper \
                --object-path /com/debmanager/Helper >/dev/null 2>&1; then
            return 0
        fi
        sleep 0.05
    done
    echo "服务启动超时"
    cat "$WORK/helper.log" >&2
    return 1
}

check() {
    local name=$1
    shift
    CHECKS=$((CHECKS + 1))
    if "$@"; then
        echo "PASS: $name"
    else
        echo "FAIL: $name"
        FAILURES=$((FAILURES + 1))
    fi
}

staging_empty() {
    if compgen -G "$STAGING/deb-*.deb" >/dev/null; then
        echo "暂存目录残留文件:" >&2
        ls -l "$STAGING" >&2
        return 1
    fi
    return 0
}

install_ok() {
    python3 "$CLIENT" "$ADDR" install "$WORK/pkg.deb" | tee "$WORK/install.log"
    return "${PIPESTATUS[0]}"
}

staged_path_used() {
    grep -qF "$STAGING/deb-" "$WORK/install.log"
}

run_reject() {
    local substring=$1
    shift
    python3 "$CLIENT" "$ADDR" reject-install "$substring" "$@"
}

# ---------- 阶段 A：允许授权，验证暂存与并发 ----------
echo "== 阶段 A：允许授权 =="
start_stack "$ALLOW_ALL" 400 "$STAGING" || exit 1

printf 'dummy deb payload\n' >"$WORK/pkg.deb"
chmod 0644 "$WORK/pkg.deb"

check "正常本地安装成功" install_ok
check "安装后暂存目录已清空" staging_empty
check "apt 参数使用暂存路径" staged_path_used

printf 'x' >"$WORK/world.deb"
chmod 0666 "$WORK/world.deb"
check "拒绝组/其他用户可写的 deb" run_reject "文件权限不安全" "$WORK/world.deb"

: >"$WORK/empty.deb"
chmod 0644 "$WORK/empty.deb"
check "拒绝空 deb 文件" run_reject "空文件" "$WORK/empty.deb"

rm -f "$WORK/big.deb"
truncate -s 5G "$WORK/big.deb"
chmod 0644 "$WORK/big.deb"
check "拒绝超过单包上限的 deb" run_reject "超过单个安装包大小上限" "$WORK/big.deb"
rm -f "$WORK/big.deb"

mkfifo "$WORK/fifo.deb"
check "拒绝 FIFO" run_reject "不是普通文件" "$WORK/fifo.deb"

check "拒绝不存在的路径" run_reject "文件不存在" "$WORK/missing.deb"
check "拒绝空参数" run_reject "未选择任何 DEB 文件"

printf 'swap payload\n' >"$WORK/swap.deb"
chmod 0644 "$WORK/swap.deb"
check "授权期间替换为符号链接被拒绝" \
    python3 "$CLIENT" "$ADDR" swap-during-auth "$WORK/swap.deb" /etc/passwd
check "O_NOFOLLOW 拒绝后无暂存残留" staging_empty

printf 'late payload\n' >"$WORK/late.deb"
chmod 0644 "$WORK/late.deb"
check "授权期间放宽权限被拒绝" \
    python3 "$CLIENT" "$ADDR" chmod-during-auth 666 "$WORK/late.deb"

check "事务进行中并发请求返回 Busy" \
    python3 "$CLIENT" "$ADDR" busy "$WORK/pkg.deb"

# ---------- 阶段 B：拒绝授权，验证 fail-closed ----------
echo "== 阶段 B：拒绝授权 =="
start_stack "" 0 "$STAGING" || exit 1
printf 'denied payload\n' >"$WORK/denied.deb"
chmod 0644 "$WORK/denied.deb"
check "未授权时拒绝安装" run_reject "未获得授权" "$WORK/denied.deb"
check "未授权时不产生事务与暂存文件" staging_empty

# ---------- 阶段 C：暂存目录不安全 ----------
echo "== 阶段 C：暂存目录权限不安全 =="
start_stack "$ALLOW_ALL" 0 "$BAD_STAGING" || exit 1
check "拒绝组/其他用户可写的暂存目录" \
    run_reject "暂存目录" "$WORK/pkg.deb"

echo "----------------------------------------"
echo "共 $CHECKS 项，失败 $FAILURES 项"
if [ "$FAILURES" -ne 0 ]; then
    echo "== helper 日志 =="
    cat "$WORK/helper.log"
    exit 1
fi
exit 0
