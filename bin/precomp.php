<?php
	define("VERBOSE", false);

	// Condacts Array
	//     array(Num_parameters, Condact_name, {Array_dependencies})
	$condacts = array(
		array(1,'AT     '), //   0
		array(1,'NOTAT  '), //   1
		array(1,'ATGT   '), //   2
		array(1,'ATLT   '), //   3
		array(1,'PRESENT'), //   4
		array(1,'ABSENT '), //   5
		array(1,'WORN   '), //   6
		array(1,'NOTWORN'), //   7
		array(1,'CARRIED'), //   8
		array(1,'NOTCARR'), //   9
		array(1,'CHANCE '), //  10
		array(1,'ZERO   '), //  11
		array(1,'NOTZERO'), //  12
		array(2,'EQ     '), //  13
		array(2,'GT     '), //  14
		array(2,'LT     '), //  15
		array(1,'ADJECT1'), //  16
		array(1,'ADVERB '), //  17
		array(2,'SFX    '), //  18
		array(1,'DESC   ', array('MESSAGE')), //  19
		array(0,'QUIT   ', array('NEWLINE')), //  20
		array(0,'END    '), //  21
		array(0,'DONE   '), //  22
		array(0,'OK     '), //  23
		array(0,'ANYKEY '), //  24
		array(1,'SAVE   '), //  25
		array(1,'LOAD   '), //  26
		array(1,'DPRINT '), //  27 *
		array(1,'DISPLAY'), //  28 *
		array(0,'CLS    '), //  29
		array(0,'DROPALL'), //  30
		array(0,'AUTOG  ', array('NEWTEXT','DONE')), //  31
		array(0,'AUTOD  ', array('NEWTEXT','DONE')), //  32
		array(0,'AUTOW  ', array('NEWTEXT','DONE')), //  33
		array(0,'AUTOR  ', array('NEWTEXT','DONE')), //  34
		array(1,'PAUSE  '), //  35
		array(2,'SYNONYM'), //  36 *
		array(1,'GOTO   '), //  37
		array(1,'MESSAGE', array('MES','NEWLINE','INKEY','CLS')), //  38
		array(1,'REMOVE ', array('NEWTEXT','DONE')), //  39
		array(1,'GET    ', array('NEWTEXT','DONE')), //  40
		array(1,'DROP   ', array('NEWTEXT','DONE')), //  41
		array(1,'WEAR   ', array('NEWTEXT','DONE')), //  42
		array(1,'DESTROY'), //  43
		array(1,'CREATE '), //  44
		array(2,'SWAP   '), //  45
		array(2,'PLACE  '), //  46
		array(1,'SET    '), //  47
		array(1,'CLEAR  '), //  48
		array(2,'PLUS   '), //  49
		array(2,'MINUS  '), //  50
		array(2,'LET    '), //  51
		array(0,'NEWLINE'), //  52
		array(1,'PRINT  '), //  53
		array(1,'SYSMESS', array('MESSAGE')), //  54
		array(2,'ISAT   '), //  55
		array(1,'SETCO  '), //  56 COPYOF in old games
		array(0,'SPACE  '), //  57 COPYOO in old games
		array(1,'HASAT  '), //  58 COPYFO in old games
		array(1,'HASNAT '), //  59 COPYFF in old games
		array(0,'LISTOBJ', array('SPACE')), //  60
		array(2,'EXTERN '), //  61
		array(0,'RAMSAVE'), //  62
		array(1,'RAMLOAD'), //  63
		array(2,'BEEP   '), //  64
		array(1,'PAPER  '), //  65
		array(1,'INK    '), //  66
		array(1,'BORDER ', array('PAPER')), //  67
		array(1,'PREP   '), //  68
		array(1,'NOUN2  '), //  69
		array(1,'ADJECT2'), //  70
		array(2,'ADD    '), //  71
		array(2,'SUB    '), //  72
		array(1,'PARSE  '), //  73
		array(1,'LISTAT ', array('NEWLINE')), //  74
		array(1,'PROCESS'), //  75
		array(2,'SAME   '), //  76
		array(1,'MES    ', array('MESSAGE')), //  77
		array(1,'WINDOW '), //  78
		array(2,'NOTEQ  '), //  79
		array(2,'NOTSAME'), //  80
		array(1,'MODE   '), //  81
		array(2,'WINAT  '), //  82
		array(2,'TIME   '), //  83
		array(1,'PICTURE'), //  84
		array(1,'DOALL  '), //  85
		array(1,'MOUSE  '), //  86
		array(2,'GFX    '), //  87
		array(2,'ISNOTAT'), //  88
		array(2,'WEIGH  '), //  89
		array(2,'PUTIN  ', array('NEWTEXT','DONE','SPACE')), //  90
		array(2,'TAKEOUT', array('NEWTEXT','DONE','SPACE')), //  91
		array(0,'NEWTEXT'), //  92
		array(2,'ABILITY'), //  93
		array(1,'WEIGHT '), //  94
		array(1,'RANDOM '), //  95
		array(2,'INPUT  '), //  96 
		array(0,'SAVEAT '), //  97
		array(0,'BACKAT '), //  98
		array(2,'PRINTAT'), //  99
		array(0,'WHATO  '), // 100
		array(1,'CALL   '), // 101
		array(1,'PUTO   '), // 102
		array(0,'NOTDONE'), // 103
		array(1,'AUTOP  ', array('NEWTEXT','DONE','SPACE')), // 104
		array(1,'AUTOT  ', array('NEWTEXT','DONE','SPACE')), // 105
		array(1,'MOVE   '), // 106
		array(2,'WINSIZE'), // 107
		array(0,'REDO   '), // 108
		array(0,'CENTRE '), // 109
		array(1,'EXIT   '), // 110
		array(0,'INKEY  '), // 111 
		array(2,'BIGGER '), // 112
		array(2,'SMALLER'), // 113 
		array(0,'ISDONE '), // 114
		array(0,'ISNDONE'), // 115 
		array(1,'SKIP   '), // 116 
		array(0,'RESTART'), // 117 
		array(1,'TAB    '), // 118
		array(2,'COPYOF '), // 119
		array(0,'dumb1  '), // 120 (according DAAD manual, internal)
		array(2,'COPYOO '), // 121 
		array(0,'dumb2  '), // 122 (according DAAD manual, internal)
		array(2,'COPYFO '), // 123
		array(0,'dumb3  '), // 124 (according DAAD manual, internal)
		array(2,'COPYFF '), // 125 
		array(2,'COPYBF '), // 126 
		array(0,'RESET  ')  // 127 
	);

	echo "==== PREPROCESS DDB CONDACTS ====\n";

	//Loading DDB file
	$ddb = file_get_contents($argv[1]);

	//Check if errors
	if (!strlen($ddb)) {
		echo "ERROR: File '$argv[1]' not exists!";
		exit;
	}
	echo "Loaded '$argv[1]' (".strlen($ddb)." bytes)\n";

	//Number of PROcess in the file
	$proNum = getByte(7);

	//Address of PROcess list
	$proLst = getWord(10);

	//Array for used and unused condacts
	$condUses = array_fill(0, 128, 0);

	//Search for CONDACTS in each PRO/Entry
	for ($p=0; $p<$proNum; $p++) {
		if (VERBOSE) echo "================================ PRO $p\n";
		$entryPos = getWord($proLst+(2*$p));
		while (getByte($entryPos)!=0) {
			if (VERBOSE) echo "PRO$p: #### Entry START\n";
			$conPos = getWord($entryPos+2);
			while (($cond=getByte($conPos))!=0xff) {
				//Remove indirection flag
				$cond &= 0x7f;
				if (VERBOSE) echo "PRO$p:        * ".$condacts[$cond][1]."\n";
				$condUses[$cond]++;
				$conPos += $condacts[$cond][0] + 1;
			}
			$entryPos += 4;
			if (VERBOSE) echo "PRO$p: #### Entry END\n";
		}
	}

	//Check condact dependencies
	do {
		$end = true;
		$usedNum = 0;
		foreach ($condUses as $key => $uses) {
			if ($uses>0) {
				$usedNum++;
				if (isset($condacts[$key][2])) {
					foreach ($condacts[$key][2] as $depName) {
						$dep = getCondactIdByName($depName);
						if ($dep<0) {
							echo "ERROR: Dependency Condact '$depName' not found!\n";
							exit;
						}
						if ($condUses[$dep]==0) $end = false;
						$condUses[$dep]++;
					}
				}
			}
		}
	} while (!$end);
	echo "Condacts in use: $usedNum (not used: ".(128-$usedNum).")\n";

	//Creating output include
	$out = "";
	foreach ($condUses as $key => $uses) {
		if ($condUses[$key]==0) {
			$out .= disableCondact($key);
		}
	}

	//Saving output
	if (!isset($argv[2])) {
		echo "ERROR: No output file...\n";
		exit;
	}
	file_put_contents($argv[2], $out);

	echo "==== PREPROCESS DONE ====\n";
	exit;


	//=================================================== FUNCTIONS

	function disableCondact($id) {
		global $condacts;
		$name = trim($condacts[$id][1]);
		return "#ifndef DISABLE_$name\n".
			   "#define DISABLE_$name\n".
			   "#define do_$name do_NOT_USED\n".
			   "#endif\n";
	}

	function getCondactIdByName($name) {
		global $condacts;
		for ($i=0; $i<count($condacts); $i++) {
			if (trim($condacts[$i][1])==$name) return $i;
		}
		return -1;
	}

	function getByte($pos) {
		global $ddb;
		return ord($ddb[$pos]);
	}

	function getWord($pos) {
		global $ddb;
		return ord($ddb[$pos++])|(ord($ddb[$pos])<<8);
	}

?>