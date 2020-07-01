#
# Shows an info panel with DAAD variables
#

namespace eval info_daad {

variable info_daad_active false
variable textheight 12
variable panel_margin 0
variable panel_columns 8
variable panel_rows [expr {int(64 / $panel_columns)}]
variable sub_panel_width 95
variable sub_panel_height [expr {$textheight + 4}]
variable panel_info

array set flaglabels { \
	0 "Dark" 1 "NOCarr" 2 "Work1" 3 "Work2" \
	4 "Stack4" 5 "Stack5" 6 "Stack6" 7 "Stack7" 8 "Stack8" 9 "Stack9" 10 "Stack10" \
	11 "Stack11" 12 "Stack12" 13 "Stack13" 14 "Stack14" 15 "Stack15" 16 "Stack16" 17 "Stack17" \
	18 "Stack18" 19 "Stack19" 20 "Stack20" 21 "Stack21" 22 "Stack22" 23 "Stack23" 24 "Stack24" \
	25 "O2Num" 26 "O2Con" 27 "O2Loc" 28 "DarkF" 29 "GFlags" 30 "Score" \
	31 "TurnsLow" 32 "TurnsHi" 33 "Verb" 34 "Noun1" 35 "Adject1" 36 "Adverb" 37 "MaxCarr" \
	38 "Player" 39 "O2Att1" 40 "O2Att2" 41 "InStream" 42 "Prompt" 43 "Prep" 44 "Noun2" \
	45 "Adject2" 46 "CPNoun" 47 "CPAdject" 48 "Time" 49 "TIFlags" 50 "DAObjNo" 51 "CONum" \
	52 "Strength" 53 "OFlags" 54 "COLoc" 55 "COWei" 56 "COCon" 57 "COWear" 58 "COAtt1" 59 "COAtt2" \
	60 "Key1" 61 "Key2" 62 "ScMode" 63 "CurWin" \
}

proc info_daad_init {} {
	variable flaglabels
	variable info_daad_active
	variable textheight
	variable panel_margin
	variable panel_rows
	variable sub_panel_width
	variable sub_panel_height
	variable panel_info

	set panel_info [dict create \
		flags [dict create \
			title "  === DAAD # System Flags ===" width 400 row 0 color 0x0000ffff \
			num "" ] \
		userflags [dict create \
			title "  === DAAD # User Flags ===" width 400 row 9 color 0x0000ffff \
			num "" ] \
	]
	set curpos [dict create 0 0 9 0]

	set maxrow 0
	set col 0
	for {set n 0} {$n < 64} {incr n} {
		set label $flaglabels($n)
		set row [expr { int(fmod($n, $panel_rows)) + 1 }]
		if {$row > $maxrow} { set maxrow $row }

		if {$n < 4} { set col 0x000000ff
		} elseif {$n < 25} { set col 0xa00000ff
		} elseif {$n < 28} { set col 0xa08000ff
		} elseif {$n < 33} { set col 0x000000ff
		} elseif {$n < 37} { set col 0x0000a0ff
		} elseif {$n < 39} { set col 0x000000ff
		} elseif {$n < 41} { set col 0xa08000ff
		} elseif {$n < 43} { set col 0x000000ff
		} elseif {$n < 48} { set col 0x0000a0ff
		} elseif {$n < 51} { set col 0x000000ff
		} elseif {$n < 52} { set col 0xa08000ff
		} elseif {$n < 53} { set col 0x000000ff
		} elseif {$n < 60} { set col 0xa08000ff
		} elseif {$n < 64} { set col 0x000000ff }

		dict append panel_info \
			$label [dict create \
				title "$n#$label" \
				num $n \
				width $sub_panel_width \
				row $row \
				color $col ]
		dict append curpos $row 0
	}

	# calc width of software item
	set full_width 0
	dict for {name info} $panel_info {
		if {[dict get $info row] == 1} {
			incr full_width [dict get $info width]
			incr full_width $panel_margin
		}
	}
	incr full_width -$panel_margin
	dict set panel_info flags width $full_width
	dict set panel_info userflags width $full_width

	for {set n 64} {$n < 256} {incr n} {
		set row [expr { int(fmod($n, 16)) + 10 }]
		dict append panel_info \
			"flag$n" [dict create \
				title "$n#" \
				num $n \
				width [expr int($full_width / 12)] \
				row $row \
				color 0x303040ff ]
		dict append curpos $row 0
	}

	# set base element
	osd create rectangle info_daad \
		-x [expr {$panel_margin * 2}] \
		-y [expr {$panel_margin * 2}] \
		-alpha 0

	# create subpanels
	dict for {name info} $panel_info {
		set row [dict get $info row]
		create_sub_panel $name \
		                 [dict get $info title] \
						 [dict get $info num ] \
						 [dict get $info color] \
		                 [dict get $info width] \
		                 [dict get $info row] \
		                 [dict get $curpos $row]
		dict incr curpos $row [expr {[dict get $info width] + $panel_margin}]
	}
}

proc create_sub_panel {name title num col width row pos} {
	variable textheight
	variable panel_margin
	variable sub_panel_height

	set value ""
	if {$num!=""} { set value "N/A" }

	osd create rectangle info_daad.$name \
		-x $pos \
		-y [expr {($sub_panel_height + $panel_margin) * $row}] \
		-h $sub_panel_height \
		-w $width \
		-rgba $col \
		-clip true
	osd create text info_daad.$name.title \
		-x 2 \
		-y 2 \
		-rgba 0xddddffff \
		-text $title \
		-size [expr {int($textheight * 0.8)}]
	osd create text info_daad.$name.value \
		-x [expr {$width - 24} ] \
		-y 2 \
		-rgba 0xffff00ff \
		-text $value \
		-size [expr {int($textheight * 0.8)}]
}

proc get_flag {num} {
	set value [debug read "memory" [expr { [peek16 0xf3ef] + $num}]]
	return $value
}

proc update_info_daad {} {
	variable info_daad_active
	variable panel_info

	dict for {name info} $panel_info {
		set num [dict get $info num]
		if {$num!=""} {
			osd configure info_daad.$name.value -text [get_flag $num]
		}
	}
	return false
}

proc toggle_info_daad {} {
	variable info_daad_active
	variable bp_num

	if {$info_daad_active} {
		set info_daad_active false
		debug remove_bp $bp_num
		osd destroy info_daad
	} else {
		set info_daad_active true
		info_daad_init
		set bp_num [debug set_bp 0xfd9a { [info_daad::update_info_daad] } ]
	}
	return ""
}

namespace export toggle_info_daad

} ;# namespace info_daad

namespace import info_daad::*
