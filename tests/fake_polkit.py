#!/usr/bin/env python3
"""最小化的 polkit Authority 替身，只实现 CheckAuthorization。

环境变量：
  FAKE_POLKIT_ALLOW       逗号分隔的允许 action，设为空表示一律拒绝
  FAKE_POLKIT_DELAY_MS    回复前的延迟，用于复现授权窗口内的并发行为
"""
import os
import sys
import time

from gi.repository import Gio, GLib

DEFAULT_ACTIONS = ",".join([
    "com.debmanager.helper.install",
    "com.debmanager.helper.remove",
    "com.debmanager.helper.update-cache",
    "com.debmanager.helper.upgrade",
])

xml = """
<node>
  <interface name="org.freedesktop.PolicyKit1.Authority">
    <method name="CheckAuthorization">
      <arg type="(sa{sv})" name="subject" direction="in"/>
      <arg type="s" name="action_id" direction="in"/>
      <arg type="a{sv}" name="details" direction="in"/>
      <arg type="u" name="flags" direction="in"/>
      <arg type="s" name="cancellation_id" direction="in"/>
      <arg type="(bba{ss})" name="result" direction="out"/>
    </method>
  </interface>
</node>
"""


def main():
    if len(sys.argv) != 2:
        raise SystemExit("用法: fake_polkit.py <bus-address>")

    allow_env = os.environ.get("FAKE_POLKIT_ALLOW")
    if allow_env is None:
        allow_env = DEFAULT_ACTIONS
    allowed = {item for item in allow_env.split(",") if item}
    delay_ms = int(os.environ.get("FAKE_POLKIT_DELAY_MS", "0"))

    connection = Gio.DBusConnection.new_for_address_sync(
        sys.argv[1],
        Gio.DBusConnectionFlags.AUTHENTICATION_CLIENT
        | Gio.DBusConnectionFlags.MESSAGE_BUS_CONNECTION,
        None, None)
    node = Gio.DBusNodeInfo.new_for_xml(xml)

    def on_call(_conn, sender, _path, _iface, _method, params, invocation):
        _subject, action, _details, flags, _cid = params.unpack()
        allowed_now = action in allowed
        print("POLKIT sender=%s action=%s flags=%s allowed=%s"
              % (sender, action, flags, allowed_now), flush=True)
        if delay_ms > 0:
            # 故意阻塞：模拟用户正在授权对话框中等待
            time.sleep(delay_ms / 1000.0)
        invocation.return_value(GLib.Variant("((bba{ss}))", ((allowed_now, False, {}),)))

    connection.register_object("/org/freedesktop/PolicyKit1/Authority",
                               node.interfaces[0], on_call, None, None)
    Gio.bus_own_name_on_connection(connection, "org.freedesktop.PolicyKit1",
                                   Gio.BusNameOwnerFlags.NONE, None, None)
    print("FAKE POLKIT READY", flush=True)
    GLib.MainLoop().run()


if __name__ == "__main__":
    main()
