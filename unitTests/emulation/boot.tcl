proc wait_until_boot {} {
  global speed
  set speed 100
}

set save_settings_on_exit off

set fullspeedwhenloading on
debug set_watchpoint write_io 0x18

set speed 9999
after time 300 wait_until_boot
