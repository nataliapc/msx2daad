proc wait_until_boot {} {
  global speed
  set speed 100
}

debug set_watchpoint write_io 0x18
set save_settings_on_exit off
set speed 9999
set fullspeedwhenloading off
after time 25 wait_until_boot
