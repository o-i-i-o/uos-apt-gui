#!/usr/bin/env python3
"""apt-helperd 安全用例客户端。

用法: security_client.py <bus-address> <command> [args...]

命令：
  install <deb>...
  reject-install <错误关键字> <deb>...
  busy <deb>                        安装事务进行中再次请求，预期 Busy
  chmod-during-auth <八进制权限> <deb>  授权等待期间放宽文件权限
  swap-during-auth <deb> <目标>       授权等待期间把文件替换为符号链接
"""
import os
import sys

from gi.repository import Gio, GLib

BUS_NAME = "com.debmanager.Helper"
BUS_PATH = "/com/debmanager/Helper"
IFACE = "com.debmanager.Helper1"


def fail(message):
    print("FAIL", message, flush=True)
    return 1


def ok(message):
    print("RESULT OK", message, flush=True)
    return 0


class Client:
    def __init__(self, address):
        self.conn = Gio.DBusConnection.new_for_address_sync(
            address,
            Gio.DBusConnectionFlags.AUTHENTICATION_CLIENT
            | Gio.DBusConnectionFlags.MESSAGE_BUS_CONNECTION,
            None, None)
        self.loop = GLib.MainLoop()
        self.finished = {}
        self.check = None
        self.conn.signal_subscribe(None, IFACE, "Finished", BUS_PATH, None,
                                   Gio.DBusSignalFlags.NONE, self._on_signal, None)
        self.conn.signal_subscribe(None, IFACE, "Output", BUS_PATH, None,
                                   Gio.DBusSignalFlags.NONE, self._on_output, None)

    def _on_signal(self, _conn, _sender, _path, _iface, _name, params, _data):
        tx, exit_code, message = params.unpack()
        print("SIGNAL Finished", tx, exit_code, message, flush=True)
        self.finished[tx] = (exit_code, message)
        if self.check:
            self.check()

    def _on_output(self, _conn, _sender, _path, _iface, _name, params, _data):
        tx, text = params.unpack()
        print("OUTPUT", tx, text.replace("\n", " ").strip(), flush=True)

    def call(self, method, params, on_reply):
        self.conn.call(BUS_NAME, BUS_PATH, IFACE, method, params, None,
                       Gio.DBusCallFlags.NONE, 60000, None, on_reply, None)

    def run(self, timeout_ms=30000):
        state = {"timed_out": False}

        def on_timeout():
            state["timed_out"] = True
            self.loop.quit()
            return False

        GLib.timeout_add(timeout_ms, on_timeout)
        self.loop.run()
        return state["timed_out"]


def install_expect_reject(client, paths, substring, setup=None):
    state = {"error": None}

    def on_reply(source, result, _data):
        try:
            reply = source.call_finish(result).unpack()
            print("UNEXPECTED_REPLY", reply, flush=True)
        except GLib.Error as exc:
            state["error"] = exc.message
            print("ERROR", exc.message, flush=True)
        client.loop.quit()

    if setup:
        setup()
    client.call("InstallLocalPackages", GLib.Variant("(as)", (paths,)), on_reply)
    if client.run():
        return fail("等待错误回复超时")
    if not state["error"]:
        return fail("预期拒绝，但调用成功")
    if substring not in state["error"]:
        return fail("错误信息未包含 %r" % substring)
    return ok(substring)


def cmd_install(client, paths):
    state = {"tx": None, "error": None}

    def complete():
        if state["tx"] in client.finished:
            client.loop.quit()

    def on_reply(source, result, _data):
        try:
            state["tx"] = source.call_finish(result).unpack()[0]
            print("REPLY", state["tx"], flush=True)
        except GLib.Error as exc:
            state["error"] = exc.message
            print("ERROR", exc.message, flush=True)
            client.loop.quit()
            return
        complete()

    client.check = complete
    client.call("InstallLocalPackages", GLib.Variant("(as)", (paths,)), on_reply)
    if client.run():
        return fail("等待安装结果超时")
    if state["error"]:
        return fail("安装失败: %s" % state["error"])
    exit_code, message = client.finished.get(state["tx"], (None, None))
    if exit_code != 0:
        return fail("事务未成功结束: %r %r" % (exit_code, message))
    return ok("安装完成")


def cmd_busy(client, path):
    state = {"tx": None, "busy": None, "error": None}

    def maybe_finish():
        if state["tx"] in client.finished and (state["busy"] or state["error"]):
            client.loop.quit()

    def on_second(source, result, _data):
        try:
            reply = source.call_finish(result).unpack()
            state["error"] = "并发调用意外成功: %r" % (reply,)
        except GLib.Error as exc:
            state["busy"] = exc.message
            print("SECOND_ERROR", exc.message, flush=True)
        maybe_finish()

    def on_reply(source, result, _data):
        try:
            state["tx"] = source.call_finish(result).unpack()[0]
            print("REPLY", state["tx"], flush=True)
        except GLib.Error as exc:
            state["error"] = exc.message
            client.loop.quit()
            return
        check()

    def check():
        if state["tx"] is not None and state["busy"] is None and state["error"] is None \
                and not state.get("second_sent"):
            state["second_sent"] = True
            client.call("UpdateCache", None, on_second)
        maybe_finish()

    client.check = check
    client.call("InstallLocalPackages", GLib.Variant("(as)", ([path],)), on_reply)
    if client.run():
        return fail("等待 Busy 用例超时")
    if state["error"]:
        return fail(state["error"])
    if "已有操作正在进行" not in (state["busy"] or ""):
        return fail("并发请求未被拒绝: %r" % (state["busy"],))
    if client.finished.get(state["tx"], (None,))[0] != 0:
        return fail("首个事务未正常完成")
    return ok("并发请求被 Busy 拒绝")


def cmd_chmod_during_auth(client, mode_text, path):
    def setup():
        def chmod():
            os.chmod(path, int(mode_text, 8))
            print("CHMOD", path, mode_text, flush=True)
            return False
        GLib.timeout_add(150, chmod)

    return install_expect_reject(client, [path], "允许其他用户写入", setup)


def cmd_swap_during_auth(client, path, target):
    def setup():
        def swap():
            os.rename(path, path + ".orig")
            os.symlink(target, path)
            print("SWAP", path, "->", target, flush=True)
            return False
        GLib.timeout_add(150, swap)

    return install_expect_reject(client, [path], "无法打开", setup)


def main(argv):
    if len(argv) < 3:
        return fail("用法: security_client.py <bus-address> <command> ...")

    client = Client(argv[1])
    command = argv[2]

    if command == "install" and len(argv) >= 4:
        return cmd_install(client, argv[3:])
    if command == "reject-install" and len(argv) >= 4:
        return install_expect_reject(client, argv[4:], argv[3])
    if command == "busy" and len(argv) == 4:
        return cmd_busy(client, argv[3])
    if command == "chmod-during-auth" and len(argv) == 5:
        return cmd_chmod_during_auth(client, argv[3], argv[4])
    if command == "swap-during-auth" and len(argv) == 5:
        return cmd_swap_during_auth(client, argv[3], argv[4])
    return fail("未知命令: %s" % " ".join(argv[2:]))


if __name__ == "__main__":
    sys.exit(main(sys.argv))
