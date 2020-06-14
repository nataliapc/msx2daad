proc wait_until_boot {} {
  global speed
  set speed 100
}

set save_settings_on_exit off
plug joyporta mouse
debug set_watchpoint write_io 0x18
set speed 9999
set fullspeedwhenloading off
after time 25 wait_until_boot
