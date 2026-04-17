set save_settings_on_exit on
set fullspeedwhenloading on
debug set_watchpoint write_io 0x18
set speed 9999

# Poll the text screen every 500ms (real time) looking for the test-completion
# marker in AUTOEXEC.BAT: echo "### FINISHED TESTS ###"
# Uses realtime instead of 'after frame' so it works at any simulation speed.
proc check_tests_done {} {
    if {[string match "*FINISHED TESTS*" [get_screen]]} {
        exit
    } else {
        after realtime 0.5 check_tests_done
    }
}

after realtime 0.5 check_tests_done
