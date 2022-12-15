proc wait_until_boot {} {
  global speed
  set speed 100
}

set save_settings_on_exit off

bind f6 cycle videosource
set videosource GFX9000
#toggle_vdp_busy
#set v9990cmdtrace true

set fullspeedwhenloading on
plug joyporta mouse
debug set_watchpoint write_io 0x18

set speed 9999
after time 25 wait_until_boot
