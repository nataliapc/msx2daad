proc wait_until_boot {} {
  global speed
  set speed 100
}

set save_settings_on_exit off
set speed 9999
set fullspeedwhenloading on
after time 25 wait_until_boot
