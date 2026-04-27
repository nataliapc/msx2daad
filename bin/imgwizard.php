#!/usr/bin/php
<?php
/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	========================================================

	## VERSION 1

	Output file format:
		Offset Size  Description
		0x0000  3    Image magic string: "IMG"
		0x0003  1    Source screen type ('5', '6', '7', '8', 'A', 'C')
		0x0004 ...   Chunks containing the image (chunk max length: 5+2043 bytes each)

	Chunk Redirect format:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type: (0:redirect)
		0x0001  2    New image location to read
		0x0003  2    Empty chunk header (filled with 0x00)

	Chunk Palette format:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type  (1:palette)
		0x0001  2    The size of the palette (32)
		0x0003  2    The size of the palette (32)
		---data---
		0x0005  32   12 bits palette data in 2 bytes format (0xRB 0x0G)

	Chunk Reset VRAM pointer format:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type: (16:ResetPointer)
		0x0001  2    Chunk data length (0x0000)
		0x0003  2    Empty chunk header (0x0000)

	Chunk ClearWindow format:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type: (17:ClearWindow)
		0x0001  2    Chunk data length (0x0000)
		0x0003  2    Empty chunk header (0x0000)

	Chunk SkipBytes format:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type: (18:SkipBytes)
		0x0001  2    Chunk data length (0x0000)
		0x0003  2    VRAM Bytes to skip

	Chunk Pause format:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type: (19:Pause)
		0x0001  2    Chunk data length (0x0000)
		0x0003  2    Time to wait in 1/50 sec units

	Chunk bitmap format:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type  (2:data_raw 3:data_rle 4:data_pletter)
		0x0001  2    Chunk data length (max: 2043 bytes)
		0x0003  2    Uncompressed data length in bytes
		---data---
		0x0005 ...   Image data (1-2043 bytes length)

	## VERSION 2

	Chunk Info format:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type: (128)
		0x0001  2    Extra header length (10)
		0x0003  2    Data length (0)
		--extra header--
		0x0005  1    Info version (1)
		0x0006  2    Chunk count
		0x0008  2    Original width (in pixels)
		0x000a  2    Original height (in pixels)
		0x000c  1    Pixel type (0: unespecified, 1: BP2 (4cols paletted), 2: BP4 (16cols paletted), 4: BD8 (256 fixed cols), etc)
		0x000d  1    Palette type (0: unespecified, 1: GRB332, 2: GRB333)
		0x000e  1    Chipset type (0: unespecified, 1: TMS9918, 2: V9938, 3: V9958, 4: V9990)

	Chunk V9938Cmd:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type  (20:V9938Cmd)
		0x0001  2    Extra header length (1)
		0x0003  2    Data length (15)
		--extra header--
		0x0005  1    Number of commands to read (max: 136)
		--data--
		0x0006  15   All paremeters packed and ready to send (15 bytes each)

	Chunk V9938CmdData:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type  (21:V9938CmdData)
		0x0001  2    Extra header length (3)
		0x0003  2    Data length (1-2040)
		--extra header--
		0x0005  1    Compressor ID
		0x0006  2    Uncompressed data length (max: 16Kb)
		---data---
		0x0008 ...   Compressed data (1-2040 bytes length)
*/

	define('CHUNK_HEAD',        5);
	define('CHUNK_SIZE',        2043);
	define('CHUNK_CMDDATA_MAX', 2040);     // CHUNK_SIZE - 3 (extra header de CmdData)
	define('CHUNK_PLETTER_MAX_UNCOMP', 11264);   // VRAM scratch zone size (PRP023)

	define('CHUNK_REDIRECT',    0);
	define('CHUNK_PALETTE',     1);
	define('CHUNK_RAW',         2);
	define('CHUNK_RLE',         3);
	define('CHUNK_PLETTER',     4);
	define('CHUNK_RESET',      16);
	define('CHUNK_CLS',        17);
	define('CHUNK_SKIP',       18);
	define('CHUNK_PAUSE',      19);
	define('CHUNK_V9938CMD',   20);
	define('CHUNK_V9938DATA',  21);
	define('CHUNK_INFO',      128);

	define('INFO_VERSION',      1);
	define('CMP_RAW',           0);
	define('CMP_RLE',           1);
	define('CMP_PLETTER',       2);
	define('CMP_ZX0',           3);     // [PRP025] V9938CmdData compressorID for ZX0

	define('VDP_HMMC',       0xF0);     // CX[L] sin transparencia → HMMC (sin op lógica)
	define('VDP_LMMC',       0xB0);     // CX[L] con transparencia → LMMC (admite op lógica)
	define('LOG_AND',        0x01);
	define('LOG_OR',         0x02);

	// COMP_ID = legacy v1 chunk type (used by 'c'/'cl'/'s'). ZX0 has NO v1 chunk
	// type — it's exclusively a v2 V9938CmdData compressor. The -1 sentinel makes
	// the legacy code path (compressChunks) reject ZX0 explicitly. The buildV9938CmdSequence
	// helper maps COMP_ID → CMP_* via fallthrough, so ZX0 still maps to CMP_ZX0=3.
	$compressors = array(
		array("raw",     "raw",   CHUNK_RAW,     "RAW"),
		array("rle",     "rle",   CHUNK_RLE,     "RLE"),
		array("pletter", "plet5", CHUNK_PLETTER, "PLETTER"),
		array("zx0",     "zx0",   -1,            "ZX0"),       // [PRP025] cx[l] only
	);
	define('RAW', 0);
	define('RLE', 1);
	define('PLETTER', 2);
	define('ZX0', 3);
	define('COMP_APP',  0);
	define('COMP_EXT',  1);
	define('COMP_ID',   2);
	define('COMP_NAME', 3);

	if (!extension_loaded('gd')) {
		die("\nERROR: The PHP \"gd/gd2\" extension must be installed...\n\n");
	}

	$appname = basename($argv[0]);
	$magic = "IMG";
	$lastPalette = false;

	if (strtoupper(substr(PHP_OS, 0, 3)) === 'WIN') {
		$compressors[PLETTER][COMP_APP] = "pletter.exe";
		$compressors[ZX0][COMP_APP]     = "zx0.exe";
	}

	if ($argc<3) {
		showSyntax();
	}
	$cmd = strtolower($argv[1]);

	// List image content
	if ($cmd == 'l' && $argc==3) {
		$fileIn = $argv[2];
		showImageContent($fileIn);
		echo "\n";
		exit;
	}

	// Redirection
	if ($cmd == 'r' && $argc==4) {
		echo "### Creating redirection file\n";
		$fileOut = $argv[2];
		$newLoc = $argv[3];
		if (!is_numeric($newLoc)) {
			echo "ERROR: Redirect location is not a integer [0-255]...\n";
			exit;
		}
		echo "    Adding redirect location to $newLoc\n";
		$out = $magic.substr($fileOut,-1).chr(CHUNK_REDIRECT).pack("v",$newLoc).chr(0xff).chr(0xff);
		file_put_contents($fileOut, $out);
		echo "### Writing $fileOut\n";
		echo "### Done.\n\n";
		exit;
	}

	// Create selected rectangle image
	if ($cmd == 's' && $argc>=6) {
		$fileIn = $argv[2];
		echo "### Loading $fileIn\n";
		if (!is_numeric($argv[3]) || !is_numeric($argv[4]) || !is_numeric($argv[5]) || !is_numeric($argv[6])) {
			echo "ERROR: x, y, w, and h must be numeric and greater than zero...\n";
			exit;
		}
		//Coords
		$x = intval($argv[3]);
		$y = intval($argv[4]);
		$w = intval($argv[5]);
		$h = intval($argv[6]);
		//Transparent color
		$transparent = -1;
		if (isset($argv[7])) {
			if (is_numeric($argv[7])) {
				$transparent = intval($argv[7]);
				echo "### Transparent color: $transparent\n";
			} else {
				echo "ERROR: Not decimal number for transparent index color...\n";
				exit;
			}
		}
		echo "### Compressor: RLE (forced)\n";
		echo "\n".
		     "WARNING: 's' command is DEPRECATED.\n".
		     "         Use 'cx' for new images (V9938 streaming, with optional --transparent-color=N).\n".
		     "         Continuing in legacy mode...\n\n";
		compressRectangle($fileIn, $x, $y, $w, $h, $transparent);
		exit;
	}

	// Create full with image with palette at last chunk
	if ($cmd == 'cl') {
		$lastPalette = true;
		$cmd = 'c';
	}

	// Create full width image
	if ($cmd == 'c' && $argc>=4) {
		$fileIn = $argv[2];
		echo "### Loading $fileIn\n";
		$lines = $argv[3];
		if (!is_numeric($lines) || $lines<=0 || $lines>212) {
			echo "ERROR: lines must be numeric and greater than zero [0-212]...\n";
			exit;
		}
		//Lines
		$lines = intval($lines);
		//Compression & Transparent color
		$compress = "";
		$transparent = -1;
		if ($argc>4) {
			if (is_numeric($argv[4])) {
				$transparent = intval($argv[4]);
			} else {
				$compress = strtoupper($argv[4]);
			}
		}
		foreach ($compressors as $comp) {
			if ($comp[COMP_NAME]==$compress) {
				$compress = $comp[COMP_NAME];
				break;
			}
		}
		if ($compress=="" && $transparent==-1) {
			echo "ERROR: Unknown compression method or not decimal number for transparent index color...\n";
			exit;
		}
		// ZX0 is v2-only — reject in legacy 'c'/'cl' path
		if ($comp[COMP_ID] < 0) {
			echo "ERROR: ZX0 compression is only supported via 'cx'/'cxl' commands, not 'c'/'cl'.\n";
			exit;
		}
		if ($transparent>=0) {
			echo "### Transparent color: $transparent\n";
			$comp = $compressors[1];
			$compress = $comp[COMP_NAME]." (forced)";
		}
		echo "### Compressor: $compress\n";
		echo "\n".
		     "WARNING: 'c'/'cl' command is DEPRECATED.\n".
		     "         Use 'cx'/'cxl' for new images (V9938 streaming, with optional --transparent-color=N).\n".
		     "         Continuing in legacy mode...\n\n";
		//Compress chunks
		compressChunks($fileIn, $lines, $comp, $transparent, NULL, NULL);
		exit;
	}

	// Create rectangle image as V9938 commands (CXL = palette at last chunk)
	if ($cmd == 'cxl') {
		$lastPalette = true;
		$cmd = 'cx';
	}
	if ($cmd == 'cx' && $argc>=7) {
		$fileIn = $argv[2];
		echo "### Loading $fileIn\n";
		if (!is_numeric($argv[3]) || !is_numeric($argv[4]) ||
		    !is_numeric($argv[5]) || !is_numeric($argv[6])) {
			echo "ERROR: x, y, w, and h must be numeric and >= 0...\n";
			exit;
		}
		$x = intval($argv[3]); $y = intval($argv[4]);
		$w = intval($argv[5]); $h = intval($argv[6]);
		// Optional positional [compressor] and optional flag --transparent-color=N
		$compress    = "RLE";   // default
		$transparent = -1;
		for ($i = 7; $i < $argc; $i++) {
			$arg = $argv[$i];
			if (strpos($arg, "--transparent-color=") === 0) {
				$val = substr($arg, strlen("--transparent-color="));
				if (!is_numeric($val) || intval($val) < 0) {
					echo "ERROR: --transparent-color requires a non-negative numeric value...\n";
					exit;
				}
				$transparent = intval($val);
			} else {
				$compress = strtoupper($arg);
			}
		}
		$comp = NULL;
		foreach ($compressors as $c) {
			if ($c[COMP_NAME]==$compress) { $comp = $c; break; }
		}
		if ($comp===NULL) {
			echo "ERROR: Unknown compression method ($compress). Use RAW/RLE/PLETTER...\n";
			exit;
		}
		if ($transparent >= 0) {
			echo "### Compressor: $compress (CX command, transparency color $transparent)\n";
		} else {
			echo "### Compressor: $compress (CX command — no transparency)\n";
		}
		compressV9938Rectangle($fileIn, $x, $y, $w, $h, $comp, $transparent);
		exit;
	}

	// Remove a CHUNK from an image
	if ($cmd == 'd' && $argc==4) {
		$id = intval($argv[3]);
		if (!is_numeric($argv[3]) || $id<0) {
			echo "ERROR: ChunkID must be numeric and greater than zero...\n";
			exit;
		}
		$fileIn = $argv[2];
		$out = showImageContent($fileIn, $id);

		echo "### Saving file\n\n";
		file_put_contents($fileIn, $out);
		exit;
	}

	// Join several IMx files in just one
	if ($cmd == 'j' && $argc>=4) {
		echo "### Joining images\n";
		$out = "";
		$i = 3;
		$magic = "IMG";
		do {
			echo "    Copying image $argv[$i]\n";
			$in = file_get_contents($argv[$i]);
			if ($magic!=substr($in, 0, strlen($magic))) {
				echo "ERROR: All files must be images and with same screen mode!\n\n";
				exit;
			}
			if ($i==3) {
				$magic = substr($in, 0, 4);
			} else {
				$in = substr($in, 4);
			}
			$out .= $in;
		} while (++$i<$argc);
		echo "### Saving file $argv[2]\n\n";
		file_put_contents($argv[2], $out);
		exit;
	}

	// Transform a SC5 image to a RGB SC10(SCA) one
	if ($cmd == '5a' && $argc==5) {
		$fileIn = $argv[2];
		$lines = intval($argv[4]);
		echo "### Loading $fileIn\n";
		$in = file_get_contents($fileIn);
		$out = "\xfe\0\0\0\xd4\0\0";
		echo "### Converting SC5 to SCA...\n";
		$pos = 7;
		$size = 128*$lines;
		while ($size-- && $pos<strlen($in)) {
			$orig = ord($in[$pos++]);
			$out .= chr(($orig & 0xf0) | 0x08);
			$out .= chr(($orig << 4) | 0x08);
		}
		echo "### Saving file $argv[3]\n\n";
		file_put_contents($argv[3], $out);
		exit;
	}

	// Transform a SC12(SCC) image to a YJK SC10(SCA) one
	if ($cmd == 'ca' && $argc==5) {
		$fileIn = $argv[2];
		$lines = intval($argv[4]);
		echo "### Loading $fileIn\n";
		$in = file_get_contents($fileIn);
		$out = "\xfe\0\0\0\xd4\0\0";
		echo "### Converting SCC to SCA...\n";
		$pos = 7;
		$size = 256*$lines;
		while ($size-- && $pos<strlen($in)) {
			$orig = ord($in[$pos++]);
			$out .= chr($orig & 0b11110111);
		}
		echo "### Saving file $argv[3]\n\n";
		file_put_contents($argv[3], $out);
		exit;
	}

	showSyntax();


	//=================================================================================
	function showSyntax()
	{
		global $appname;
		
		echo "\n".
			 "IMGWIZARD v1.4.0 for MSX2DAAD\n".
			 "===================================================================\n".
			 "A tool to create and manage MSX image files in several screen modes\n".
			 "to be used by MSX2DAAD engine.\n\n".
			 "L) List image chunks:\n".
			 "    $appname l <fileIn.IM?>\n\n".
			 "CX) Create a rectangle image as V9938 commands (CXL = palette last):\n".
			 "    $appname cx[l] <fileIn.SC?> <x> <y> <w> <h> [compressor] [--transparent-color=N]\n\n".
			 "C) [DEPRECATED] Create an image IMx (CL - Create the palette at last chunk):\n".
			 "    $appname c[l] <fileIn.SC?> <lines> [compressor | transparent_color]\n\n".
			 "S) [DEPRECATED] Create an image from a rectangle:\n".
			 "    $appname s <fileIn.SC?> <x> <y> <w> <h> [transparent_color]\n\n".
			 "R) Create a location redirection:\n".
			 "    $appname r <fileOut.IM?> <target_loc>\n\n".
			 "D) Remove a CHUNK from an image:\n".
			 "    $appname d <fileIn.IM?> <chunk_id>\n\n".
			 "J) Join several IMx files in just one:\n".
			 "    $appname j <fileOut.IM?> <fileIn1.IM?> [fileIn2.IM?] [fileIn3] ...\n\n".
			 "5A) Transform a SC5 image to a RGB SC10(SCA) one:\n".
			 "    $appname 5a <fileIn.SC5> <fileOut.SCA> <lines>\n\n".
			 "CA) Transform a SC12(SCC) image to a YJK SC10(SCA) one:\n".
			 "    $appname ca <fileIn.SCC> <fileOut.SCA> <lines>\n\n".
			 " <fileIn>      Input file in format SCx (SC5/SC6/SC7/SC8/SCA/SCC)\n".
			 "               Palette can be inside SCx file or PL5 PL6 PL7 files.\n".
			 " <lines>       Image lines to get from input file.\n".
			 " [compressor]  Compression type: RAW, RLE, PLETTER or ZX0.\n".
			 "                 RAW: no compression but fastest load.\n".
			 "                 RLE: light compression but fast load (default).\n".
			 "                 PLETTER: high compression but slow.\n".
			 "                 ZX0: highest compression but slowest.\n".
			 " [transparent] Optional: the color index that will become transparent (decimal).\n".
			 "               Compression is forced to RLE for legacy 's'/'c' commands.\n".
			 " --transparent-color=N\n".
			 "               Optional flag for 'cx[l]': color index N treated as transparent.\n".
			 "               Generates 2-pass LMMC AND+OR streaming instead of 1-pass HMMC.\n".
			 "               Supported in: SC5 (0..15), SC6 (0..3), SC7 (0..15), SC8 (0..255),\n".
			 "                             SC10 (0..15 paletted; YJK pixels auto-preserved).\n".
			 "               NOT supported in SC12 (pure YJK, no per-pixel A flag).\n".
			 " <target_loc>  Target location number to redirect to.\n".
			 "                 ex: a 12 redirects to image 012.IMx\n".
			 "\n".
			 "Example: $appname cx image.sc8 0 0 256 96 rle\n".
			 "\n";
		exit(1);
	}

	//=================================================================================
	function buildInfoChunk($scr, $chunkCount, $width, $height)
	{
		static $pixel   = ['5'=>2,'6'=>1,'7'=>2,'8'=>4,'A'=>7,'C'=>6];
		static $palette = ['5'=>2,'6'=>2,'7'=>2,'8'=>1,'A'=>1,'C'=>0];
		static $chipset = ['5'=>2,'6'=>2,'7'=>2,'8'=>2,'A'=>3,'C'=>2];

		$scr = strtoupper($scr);
		return chr(CHUNK_INFO)
		     . pack("vv", 10, 0)                             // extraHeaderSize=10, dataSize=0
		     . chr(INFO_VERSION)                             // infoVersion=1
		     . pack("vvv", $chunkCount, $width, $height)     // chunkCount, originalWidth, originalHeight
		     . chr($pixel[$scr]   ?? 0)
		     . chr($palette[$scr] ?? 0)
		     . chr($chipset[$scr] ?? 0);
	}

	//=================================================================================
	// Count chunks in a binary IMx body (without magic+screenMode header).
	// Walks the type/extraHeaderSize/dataSize tuple and advances to the next chunk.
	// Returns the number of chunks found.
	function countChunks($body)
	{
		$count = 0;
		$pos   = 0;
		$len   = strlen($body);
		while ($pos + 5 <= $len) {
			$sin  = unpack("v", substr($body, $pos+1, 2))[1];   // SizeOut/extraHeaderSize for v2 — bytes after 5-hdr
			$sout = unpack("v", substr($body, $pos+3, 2))[1];   // SizeIn/dataSize
			$type = ord($body[$pos]);
			// v1 chunks (PALETTE/RAW/RLE/PLETTER): the data length is sin
			// v2 chunks (INFO/CMD/CMDDATA): bytes after 5-hdr = extraHeaderSize + dataSize
			if ($type==CHUNK_INFO || $type==CHUNK_V9938CMD || $type==CHUNK_V9938DATA) {
				$pos += 5 + $sin + $sout;
			} else {
				$pos += 5 + $sin;
			}
			$count++;
		}
		return $count;
	}

	//=================================================================================
	// Extract a rectangle from a SCx body (raw bytes, no header) into a contiguous buffer.
	function sliceRect($in, $x, $y, $w, $h, $pixelsByte, $bytesLine)
	{
		$xb = intval($x / $pixelsByte);
		$wb = intval(round($w / $pixelsByte));
		$out = "";
		for ($i = 0; $i < $h; $i++) {
			$out .= substr($in, $xb + ($y+$i)*$bytesLine, $wb);
		}
		return $out;
	}

	//=================================================================================
	function convertPalette89($pal)
	{
		$out = "";
		for ($i=0; $i<16; $i++) {
			$r = ord($pal[0x29+$i*3]);
			$g = ord($pal[0x29+$i*3+1]);
			$b = ord($pal[0x29+$i*3+2]);
			$grb = chr((($r & 0x0F)<<4) | ($b & 0x0F)).chr($g & 0x0F);
			$out .= $grb;
		}
		return $out;
	}

	//=================================================================================
	function showImageContent($fileIn, $removeId=FALSE)
	{
		global $magic;

		echo "### Reading file $fileIn\n";
		$in = file_get_contents($fileIn);

		//Magic & Screen type
		if (substr($in, 0, 3)!=$magic) {
			echo "ERROR: bad file type...\n";
			exit;
		}
		$out = substr($in, 0, 4);

		//Screen mode
		$scr = strtoupper(substr($in, 3, 1));
		if (($scr<'5' || $scr>'8') && $scr!='A' && $scr!='C') {
			echo "ERROR: bad screen mode ['$scr']...\n";
			exit;
		}
		echo "### Mode SCREEN ".hexdec($scr)."\n";

		// Chunks
		$pos = 4;
		$totalRaw = 0;
		$totalComp = 0;
		$id = 1;
		while ($pos < strlen($in)) {
			list($type,$sin,$sout) = array_values(unpack('CType/vSizeOut/vSizeIn', substr($in, $pos, 5)));
			$size = 5;
			switch ($type) {
				case CHUNK_REDIRECT:
					echo "    CHUNK $id: Redirect -> ".strpad($sin, 3, '0').".IM$scr\n";
					break;
				case CHUNK_PALETTE:
					echo "    CHUNK $id: RGB333 Palette $sout bytes\n";
					$size += $sin;
					break;
				case CHUNK_RAW:
					echo "    CHUNK $id: RAW Data: $sout bytes\n";
					$size += $sin;
					break;
				case CHUNK_RLE:
					echo "    CHUNK $id: RLE Data: $sout bytes ($sin bytes compressed) [".number_format($sin/$sout*100,1,'.','')."%]\n";
					$size += $sin;
					break;
				case CHUNK_PLETTER:
					echo "    CHUNK $id: PLETTER Data: $sout bytes ($sin bytes compressed) [".number_format($sin/$sout*100,1,'.','')."%]\n";
					$size += $sin;
					break;
				case CHUNK_RESET:
					echo "    CHUNK $id: CMD:ResetPointer\n";
					$size += $sin;
					break;
				case CHUNK_CLS:
					echo "    CHUNK $id: CMD:ClearWindow\n";
					$size += $sin;
					break;
				case CHUNK_SKIP:
					echo "    CHUNK $id: CMD:SkipVRAMBytes ($sout bytes)\n";
					$size += $sin;
					break;
				case CHUNK_PAUSE:
					echo "    CHUNK $id: CMD:Pause ($sout/50 seconds)\n";
					$size += $sin;
					break;
				case CHUNK_INFO:
					// $sin = extraHeaderSize (10), $sout = dataSize (0)
					$pixelTypeNames = [0=>'Unspecified', 1=>'BP2 (4 cols paletted)', 2=>'BP4 (16 cols paletted)', 4=>'BD8 (256 fixed cols)'];
					$paletteTypeNames = [0=>'Unspecified', 1=>'GRB332', 2=>'GRB333'];
					$chipsetTypeNames = [0=>'Unspecified', 1=>'TMS9918', 2=>'V9938', 3=>'V9958', 4=>'V9990'];
					$extra = substr($in, $pos+5, $sin);
					$ver   = ord($extra[0]);
					$cnt   = unpack("v", substr($extra, 1, 2))[1];
					$wInf  = unpack("v", substr($extra, 3, 2))[1];
					$hInf  = unpack("v", substr($extra, 5, 2))[1];
					$px    = $pixelTypeNames[ord($extra[7])];
					$pl = $paletteTypeNames[ord($extra[8])];
					$cs = $chipsetTypeNames[ord($extra[9])];
					echo "    CHUNK $id: INFO v$ver\n".
					     "        Chunks count:    $cnt\n".
						 "        Original size:   $wInf x $hInf pixels\n".
						 "        Pixel Type:      $px\n".
						 "        Palette Type:    $pl\n".
						 "        Chipset Type:    $cs\n";
					$size += $sin + $sout;
					break;
				case CHUNK_V9938CMD:
					$n = ord(substr($in, $pos+5, 1));
					echo "    CHUNK $id: V9938Cmd ($n commands, ".$sout." bytes)\n";
					$cmdNames = [
						0x00 => 'STOP',  0x40 => 'POINT', 0x50 => 'PSET',
						0x60 => 'SRCH',  0x70 => 'LINE',  0x80 => 'LMMV',
						0x90 => 'LMMM',  0xA0 => 'LMCM',  0xB0 => 'LMMC',
						0xC0 => 'HMMV',  0xD0 => 'HMMM',  0xE0 => 'YMMM', 0xF0 => 'HMMC',
					];
					for ($k = 0; $k < $n; $k++) {
						$off = $pos + 6 + $k*15;
						$rs  = unpack("vsx/vsy/vdx/vdy/vnx/vny/Cclr/Carg/Ccmd", substr($in, $off, 15));
						$opName = $cmdNames[$rs['cmd'] & 0xF0] ?? sprintf('?(0x%02X)', $rs['cmd']);
						$logic  = $rs['cmd'] & 0x0F;
						printf("        cmd %2d: %s%s SX=%d SY=%d DX=%d DY=%d NX=%d NY=%d CLR=0x%02X ARG=0x%02X\n",
							$k+1, $opName, $logic ? sprintf('|0x%X', $logic) : '',
							$rs['sx'], $rs['sy'], $rs['dx'], $rs['dy'], $rs['nx'], $rs['ny'], $rs['clr'], $rs['arg']);
					}
					$size += $sin + $sout;
					break;
				case CHUNK_V9938DATA:
					$extra = substr($in, $pos+5, 3);
					$compID = ord($extra[0]);
					$uncomp = unpack("v", substr($extra, 1, 2))[1];
					$compName = ['RAW','RLE','PLETTER','ZX0'][$compID] ?? '?';
					echo "    CHUNK $id: V9938CmdData [$compName] $uncomp bytes ($sout comp)\n";
					$size += $sin + $sout;
					break;
				default:
					echo "    CHUNK $id: UNKNOWN CHUNK TYPE!!!! [**Aborted**]\n\n";
					exit;
			}
			if ($type==CHUNK_V9938DATA) {
				$totalRaw  += $uncomp;       // uncompressedSize del extra header
				$totalComp += $sout;         // dataSize = payload comprimido
			} else if ($type==CHUNK_INFO || $type==CHUNK_V9938CMD) {
				// INFO y V9938Cmd no son datos de imagen — no contribuyen al ratio
			} else if ($type != CHUNK_REDIRECT) {
				$totalRaw  += $sout;
				$totalComp += $sin;
			}
			if ($id===$removeId) {
				echo "--REMOVED CHUNK $id--\n";
			} else {
				$out .= substr($in, $pos, $size);
			}
			$pos += $size;
			$id++;
		}
		if ($removeId>=$id) {
			echo "!!!WARNING: CHUNK $id NOT FOUND!!!\n";
		}
		if ($type != CHUNK_REDIRECT && $removeId===FALSE) {
			$compSize = $totalRaw==0 ? 0 : $totalComp/$totalRaw*100;
			echo "### Original size:   $totalRaw bytes\n";
			echo "### Compressed size: $totalComp bytes [".number_format($compSize,1,'.','')."%]\n";
		}
		echo "### End of file\n";
		return $out;
	}

	//=================================================================================
	function addPalette($file, $fileIn, $scr, $pal=NULL, $paper=NULL, $ink=NULL)
	{
		$out = "";
		if ($pal===NULL) {
			$filePalette = substr($file, 0, strlen($file)-3)."PL".$scr;
			if (!file_exists($filePalette)) { $filePalette = substr($file, 0, strlen($file)-3)."PAL"; }
			if (!file_exists($filePalette)) { $filePalette = ""; }
			if (!file_exists($filePalette)) {
				if ($scr=="5" || $scr=="6") {
					if (strlen($fileIn) >= 0x7680+32) {
						$filePalette = $file;
						$pal = substr($fileIn, 0x7680, 32);
					}
				} else {
					if (strlen($fileIn) >= 0xfa80+32) {
						$filePalette = $file;
						$pal = substr($fileIn, 0xfa80, 32);
					}
				}
			}
		}

		if ($pal!==NULL || $filePalette!="") {	// Add palette chunk
			echo "### Adding image palette from file '$filePalette'\n".
			     "    #CHUNK  1 RGB333 Palette 32 bytes\n";
			if ($pal===NULL) {
				$pal = file_get_contents($filePalette);
			}
			if (strlen($pal)==89) {
				$pal = convertPalette89($pal);
			} elseif (strlen($pal)!=32) {
				die("\nERROR: Unknown Palette format!\n\n");
			}

			if ($paper!==NULL) {
				$aux0 = $pal[0];
				$aux1 = $pal[1];
				$pal[0] = $pal[$paper*2];
				$pal[1] = $pal[$paper*2+1];
				$pal[$paper*2] = $aux0;
				$pal[$paper*2+1] = $aux1;
			}
			if ($ink!==NULL) {
				$aux0 = $pal[15*2];
				$aux1 = $pal[15*2+1];
				$pal[15*2] = $pal[$ink*2];
				$pal[15*2+1] = $pal[$ink*2+1];
				$pal[$ink*2] = $aux0;
				$pal[$ink*2+1] = $aux1;
			}

			$out = chr(CHUNK_PALETTE).pack("vv",32,32).$pal;
		} else {
			echo "### Palette not found\n";
		}
		return $out;
	}

	//=================================================================================
	function checkScreemMode($fileIn)
	{
		$scr = strtoupper(substr($fileIn, -1));
		if (($scr<'5' || $scr>'8') && $scr!='A' && $scr!='C') {
			die("\nERROR: bad screen mode ['$scr']...\n\n");
		}
		return $scr;
	}

	//=================================================================================
	function checkPalettedColors($in, $scr)
	{
		$bpp = 4;					//Bits per pixel
		if ($scr==6) { $bpp = 2; }
		if ($scr==8) { $bpp = 8; }
		$ppb = 8/$bpp;				//Pixels x Byte
		$mask = pow(2, $bpp)-1;

		$colors = array_fill(0, $mask+1, 0);

		for ($i=0; $i<strlen($in); $i++) {
			$byte = ord($in[$i]);
			for ($p=0; $p<$ppb; $p++) {
				$colors[$byte & $mask]++;
				$byte >>= $bpp;
			}
		}

		$paper = $ink = NULL;
		if ($scr!=8) {
			if ($colors[0]>0) {
				echo "WARNING: PAPER Color (index 0) is used in the image!\n";
/*				$unused = firstUnusedColor($colors);
				if ($unused===FALSE) {
					echo "ERROR: PAPER Color is used and not unused colors to switch them!\n\n";
					exit;
				}
				$in = swapColors($in, 0, $unused, $bpp);
				$paper = $unused;
				$colors[$paper] = $colors[0];
				$colors[0] = 0;*/
			}
			if ($colors[$mask]>0) {
				echo "WARNING: INK Color (index $mask) is used in the image!\n";
/*				$unused = firstUnusedColor($colors);
				if ($unused===FALSE) {
					echo "ERROR: INK Color is used and not unused colors to switch them!\n\n";
					exit;
				}
				$in = swapColors($in, $mask, $unused, $bpp);
				$ink = $unused;
				$colors[$ink] = $colors[$mask];
				$colors[$mask] = 0;*/
			}
		}
		return array($in, $paper, $ink);
	}

	//=================================================================================
	function swapColors($in, $from, $to, $bpp)
	{
		$ppb = 8/$bpp;				//Pixels x Byte
		$mask = pow(2, $bpp)-1;
		$num = 0;

		for ($i=0; $i<strlen($in); $i++) {
			$byte = ord($in[$i]);
			for ($p=0; $p<$ppb; $p++) {
				if ((($byte >> $bpp*$p) & $mask) == $from) {
					$num++;
					$mbit = $mask << ($p*$bpp);
					$byte &= $mbit ^ 255;
					$byte |= $to << ($p*$bpp);
					$in[$i] = chr($byte);
				}
			}
		}
		echo "WARNING: Swap color $from and color $to ($num pixels changed)\n";

		return $in;
	}

	//=================================================================================
	function firstUnusedColor($colors)
	{
		for ($i=1; $i<count($colors)-1; $i++) {
			if ($colors[$i]==0) {
				return $i;
			}
		}
		return FALSE;
	}

	//=================================================================================
	function getTransparentColorByte($transparent, $scr)
	{
		if ($transparent<0) {
			return $transparent;
		}
		if ($scr==5 || $scr==7) {
			$transparent = $transparent&0x0f | (($transparent&0x0f)<<4);
		}
		if ($scr==6) {
			$transparent = $transparent&0x03;
			$transparent = $transparent | ($transparent<<2) | ($transparent<<4) | ($transparent<<6);
		}
		if ($scr=='C') {
			echo "SCREEN 12 images can't support transparency at this time...\n";
			exit;
		}
		return $transparent;
	}

	//=================================================================================
	function compressRectangle($file, $x, $y, $w, $h, $transparent=-1, $in=NULL, $pal=NULL)
	{
		global $magic;
		$id = 1;                                    // INFO ya cuenta como chunk 1

		// Check screen mode
		$out = $magic;
		$scr = checkScreemMode($file);
		echo "### Mode SCREEN ".hexdec($scr)."\n";
		if (hexdec($scr)>=10 && ($x%4!=0 || $w%4!=0)) {
			die("\nERROR: SCREEN 10/12 needs 'x' and 'w' input to be multiple of 4...\n\n");
		}
		$out .= $scr;
		echo "### Rectangle Start:($x, $y) Width:($w, $h)\n";

		// Reservar 15 bytes para el INFO (rellenado al final con chunkCount real)
		$infoPos = strlen($out);
		$out .= str_repeat("\0", 15);

		// Transparent color-byte
		$transparent = getTransparentColorByte($transparent, $scr);
		// Read file
		if ($in===NULL)
			$in = @file_get_contents($file);
			$in = substr($in, 7);
		if ($in===FALSE) {
			echo "File not found...\n";
			exit;
		}

		// Add palette to paletted screen modes
		if (hexdec($scr) < 8 && !$lastPalette) {
			list($in, $paper, $ink) = checkPalettedColors($in, $scr);
			$aux = addPalette($file, $in, $scr, $pal);
			if ($aux!="") {
				$id++;
				$out .= $aux;
			}
		}

		// Add CHUNK_CLS if not transparent image
		if ($transparent<0) {
			$out .= pack("cvv", CHUNK_CLS, 0, 0);
			echo "    #CHUNK ".strpad($id++,2)." CMD CLS (clear window)\n";
		}

		// Bytes each Row in screen modes
		$pixelsByte = array(0,0,0,0,0,2,4,2,1,'A'=>1,'C'=>1);
		$bytesLine = array(0,0,0,0,0,128,128,256,256,'A'=>256,'C'=>256);

		$i = 0;

		$wb = intval(round($w / $pixelsByte[$scr]));
		$fullSize = $h * $wb;
		while ($i < $h) {
			$j = $h - $i;
			for (;;) {
				$compOut = rle_encode_selection($in, $x, $y+$i, $w, $j, $transparent, $pixelsByte[$scr], $bytesLine[$scr]);
				if (strlen($compOut)<=CHUNK_SIZE) break;
				$j--;
			}
			$sizeIn = $j*$bytesLine[$scr];
			$sizeOut = strlen($compOut);
			echo "    #CHUNK ".strpad($id,2)." sizeIn: ".($j*$wb)." bytes (out: $sizeOut bytes)\n";
			$out .= pack("cvv", CHUNK_RLE, $sizeOut, $sizeIn).$compOut;

			$i += $j;
			$id++;
		}

		// Add palette to paletted screen modes
		if (hexdec($scr) < 8 && !$lastPalette) {
			list($in, $paper, $ink) = checkPalettedColors($in, $scr);
			$aux = addPalette($file, $in, $scr, $pal);
			if ($aux!="") {
				$id++;
				$out .= $aux;
			}
		}

		// Rellenar el INFO con el chunkCount real (originalWidth=$w, originalHeight=$h)
		$chunkCount = countChunks(substr($out, $infoPos+15)) + 1;   // +1 para el propio INFO
		$infoBin    = buildInfoChunk($scr, $chunkCount, $w, $h);
		$out        = substr_replace($out, $infoBin, $infoPos, 15);

		// Show result
		echo "    In: ".$fullSize." bytes\n    Out: ".(strlen($out)+7)." bytes [".number_format(strlen($out)/$fullSize*100,1,'.','')."%]\n";
		$file = basename($file);
		$fileOut = substr($file, 0, strlen($file)-3)."IM".$scr;

		// Write put file
		echo "### Writing $fileOut\n";
		file_put_contents($fileOut, $out);
		echo "### Done\n\n";
	}

	//=================================================================================
	function compressChunks($file, $lines, $comp, $transparent=-1, $in=NULL, $pal=NULL)
	{
		global $magic;
		global $lastPalette;
		$id = 1;                                     // INFO ya cuenta como chunk 1
		$tmp = tempnam(sys_get_temp_dir(), 'imgwiz');

		// Bytes each Row in screen modes
		$width = array(0,0,0,0,0,128,128,256,256,'A'=>256,'C'=>256);

		// Check screen mode
		$out = $magic;
		$scr = checkScreemMode($file);
		echo "### Mode SCREEN ".hexdec($scr)."\n";
		$out .= $scr;
		echo "### Lines $lines\n";

		// Reservar 15 bytes para el INFO (rellenado al final con chunkCount real)
		$infoPos = strlen($out);
		$out .= str_repeat("\0", 15);

		// Transparent color-byte
		$transparent = getTransparentColorByte($transparent, $scr);

		// Read file
		if ($in===NULL) {
			$in = @file_get_contents($file);
			$in = substr($in, 7);
		}
		if ($in===FALSE) {
			echo "File not found...\n";
			exit;
		}

		// Add palette to paletted screen modes
		$rest = substr($in, $width[$scr]*$lines);
		$in = substr($in, 0, $width[$scr]*$lines);
		if (hexdec($scr) < 8 && !$lastPalette) {
			list($in, $paper, $ink) = checkPalettedColors($in, $scr);
			$aux = addPalette($file, $in.$rest, $scr, $pal);
			if ($aux!="") {
				$id++;
				$out .= $aux;
			}
		}

		// Add CHUNK_CLS if not transparent image
		if ($transparent<0) {
			$out .= pack("cvv", CHUNK_CLS, 0, 0);
			echo "    #CHUNK ".strpad($id++,2)." CMD CLS (clear window)\n";
		}

		$pos = 0;

		$fullSize = strlen($in);
		while ($pos < strlen($in)) {
			$sizeIn = CHUNK_SIZE;
			$sizeDelta = intval(CHUNK_SIZE/2);
			$end = false;
			do {
				$sizeOut = compress($tmp, $in, $pos, $sizeIn, $comp, $transparent);
				if ($sizeIn+$pos >= $fullSize && $sizeOut<=CHUNK_SIZE) {
					$sizeIn = $fullSize-$pos;
					$sizeOut = compress($tmp, $in, $pos, $sizeIn, $comp, $transparent);
					$end = true;
				} else
				if ($sizeOut < CHUNK_SIZE-1) {
					if ($sizeDelta > 0) {
						$sizeIn += $sizeDelta;
					} else {
						$end = true;
					}
				} else {
					if ($sizeOut > CHUNK_SIZE) {
						$sizeIn -= $sizeDelta;
						$sizeDelta = intval($sizeDelta*0.95);
					} else
						$end = true;
				}
				echo "\r\x1b[2K    #CHUNK ".strpad($id,2)." (".strpad($pos,5)."): sizeIn: $sizeIn bytes (out: $sizeOut bytes)";
			} while (!$end);
			$out .= pack("cvv", $comp[COMP_ID], $sizeOut, $sizeIn).file_get_contents($tmp.'.'.$comp[COMP_EXT]);
			echo "\n";
			$id++;
			$pos += $sizeIn;
		}

		// Add palette to paletted screen modes at last chunk
		if (hexdec($scr) < 8 && $lastPalette) {
			list($in, $paper, $ink) = checkPalettedColors($in, $scr);
			$aux = addPalette($file, $in.$rest, $scr, $pal);
			if ($aux!="") {
				$id++;
				$out .= $aux;
			}
		}

		// Rellenar el INFO con el chunkCount real (originalWidth=ancho del modo, originalHeight=lines)
		$chunkCount = countChunks(substr($out, $infoPos+15)) + 1;   // +1 para el propio INFO
		$infoBin    = buildInfoChunk($scr, $chunkCount, $width[$scr], $lines);
		$out        = substr_replace($out, $infoBin, $infoPos, 15);

		// Show result
		echo "    In: ".$fullSize." bytes\n    Out: ".(strlen($out)+7)." bytes [".number_format(strlen($out)/$fullSize*100,1,'.','')."%]\n";
		$file = basename($file);
		$fileOut = substr($file, 0, strlen($file)-3)."IM".$scr;

		// Write put file
		echo "### Writing $fileOut\n";
		file_put_contents($fileOut, $out);

		// Delete temp files
		@unlink($tmp);
		@unlink($tmp.'.'.$comp[COMP_EXT]);
		echo "### Done\n\n";
	}

	//=================================================================================
	// Process a packed-pixel rectangle to produce mask + image buffers (1 byte/pixel).
	//
	// $rectData     : packed rectangle bytes (output of sliceRect).
	// $w, $h        : rectangle size in pixels.
	// $sup          : uppercase screen mode char ('5','6','7','8').
	// $transparent  : color index marking the transparent pixel.
	//
	// Returns [$maskBuffer, $imageBuffer], both w*h bytes long, 1 byte per pixel.
	//   - mask byte  = full_bits (0xFF / 0x0F / 0x03) for transparent pixels (preserve dest)
	//   - mask byte  = 0x00 for visible pixels (clear dest before OR pass)
	//   - image byte = pixel color for visible pixels, 0 for transparent
	//
	// MSX byte→pixel mapping: high bits = leftmost pixel, low bits = rightmost.
	function processTransparency($rectData, $w, $h, $sup, $transparent)
	{
		// SCA (SC10, YJK+YAE) requires a different algorithm — see PRP026.
		if ($sup === 'A') {
			return processTransparencySCA($rectData, $transparent);
		}

		$bppMode = ['5'=>4,'6'=>2,'7'=>4,'8'=>8];
		$bpp = $bppMode[$sup];
		$ppb = intval(8 / $bpp);
		$fullBits = (1 << $bpp) - 1;        // 0x0F SC5/7, 0x03 SC6, 0xFF SC8

		$bytesPerLine = intval($w / $ppb);
		$mask  = '';
		$image = '';
		$transCount = 0;

		for ($row = 0; $row < $h; $row++) {
			$rowBase = $row * $bytesPerLine;
			for ($col = 0; $col < $bytesPerLine; $col++) {
				$byte = ord($rectData[$rowBase + $col]);
				// Iterate pixels left-to-right within byte (high bits first).
				for ($p = 0; $p < $ppb; $p++) {
					$shift = ($ppb - 1 - $p) * $bpp;
					$pixel = ($byte >> $shift) & $fullBits;
					if ($pixel === $transparent) {
						$mask  .= chr($fullBits);
						$image .= chr(0);
						$transCount++;
					} else {
						$mask  .= chr(0);
						$image .= chr($pixel);
					}
				}
			}
		}
		if ($transCount === 0) {
			echo "WARNING: no pixels with color $transparent found in rectangle. Mask is empty.\n";
		}
		return [$mask, $image];
	}

	//=================================================================================
	// PRP026 — SCREEN 10 (SCA, YJK+YAE) transparency.
	//
	// SCA byte layout: bits 7-4 = Y or palette index, bit 3 = A flag (0:YJK, 1:paletted),
	// bits 2-0 = K/J chroma contribution shared across the 4-byte aligned group.
	//
	// Transparency rules:
	//   A=0 (YJK source pixel)         → preserve dest fully (mask=0xFF, image=0x00)
	//   A=1, idx == transparent_color  → preserve dest fully (mask=0xFF, image=0x00)
	//   A=1, idx != transparent_color  → overwrite Y/idx+A bits (7-3), preserve dest's
	//                                     bits 2-0 to keep YJK group chroma intact:
	//                                       mask  = 0x07
	//                                       image = byte & 0xF8
	function processTransparencySCA($rectData, $transparent)
	{
		$mask  = '';
		$image = '';
		$yjkCount = 0;
		$transCount = 0;
		$visibleCount = 0;
		$totalBytes = strlen($rectData);

		for ($i = 0; $i < $totalBytes; $i++) {
			$byte = ord($rectData[$i]);
			if (($byte & 0x08) === 0) {
				$mask  .= chr(0xFF);
				$image .= chr(0x00);
				$yjkCount++;
			} else {
				$paletteIdx = ($byte >> 4) & 0x0F;
				if ($paletteIdx === $transparent) {
					$mask  .= chr(0xFF);
					$image .= chr(0x00);
					$transCount++;
				} else {
					$mask  .= chr(0x07);
					$image .= chr($byte & 0xF8);
					$visibleCount++;
				}
			}
		}
		echo "    [SCA stats: $visibleCount visible, $transCount paletted-transparent, $yjkCount YJK auto-preserved]\n";
		if ($visibleCount === 0) {
			echo "WARNING: no visible paletted pixels in rectangle.\n";
		}
		return [$mask, $image];
	}

	//=================================================================================
	// Compress $buffer into V9938CmdData chunks and emit one V9938Cmd that covers the
	// rectangle (DX,DY,NX,NY) with the given $opcode. Returns ['cmd' => bin, 'data' => bin].
	//
	// Units of (DX, NX) depend on the opcode:
	//   - HMMC (byte mode):   pass byte coords (DX_bytes = X/ppb, NX_bytes = W/ppb).
	//   - LMMC (dot mode):    pass pixel coords directly (DX = X, NX = W).
	function buildV9938CmdSequence($buffer, $dx, $dy, $nx, $ny, $comp, $opcode)
	{
		$origLen = strlen($buffer);
		if ($origLen === 0) {
			die("\nERROR: buildV9938CmdSequence called with empty buffer.\n\n");
		}

		// V9938 HMMC/LMMC consume R#44 (CLR) as the FIRST pixel byte at command
		// dispatch (per Grauw / V9938 spec). To process exactly NX*NY pixels:
		//   - Preload R#44 with data[0] via the V9938Cmd's CLR field.
		//   - Stream only data[1..N-1] via #9B → pixels 1..N-1.
		// This must use the UNCOMPRESSED first byte (real pixel value), not the
		// first byte of a compressed payload (which for RLE/Pletter is a control
		// byte, not pixel data).
		$firstPixelByte = ord($buffer[0]);
		$streamBuffer   = substr($buffer, 1);
		$totalUncomp    = strlen($streamBuffer);

		$dataChunks = [];
		// Edge case: 1-pixel rectangle → no streaming, just R#44 dispatch.
		if ($totalUncomp > 0) {
			// Cap de crecimiento de tamaño uncompressed por chunk:
			//   PLETTER: límite por zona scratch VRAM del engine (PRP023).
			//   RAW/RLE: sin cap, el límite efectivo lo impone CHUNK_CMDDATA_MAX.
			$capGrow = ($comp[COMP_ID]==CHUNK_PLETTER) ? CHUNK_PLETTER_MAX_UNCOMP : ($totalUncomp);
			$tmp = tempnam(sys_get_temp_dir(), 'imgwiz');
			$pos = 0;
			while ($pos < $totalUncomp) {
				$sizeIn    = min($totalUncomp - $pos, CHUNK_SIZE);
				$sizeDelta = intval(CHUNK_SIZE / 2);
				$end = false;
				do {
					$sizeOut = compress($tmp, $streamBuffer, $pos, $sizeIn, $comp, -1);
					if ($pos + $sizeIn >= $totalUncomp && $sizeOut <= CHUNK_CMDDATA_MAX) {
						$end = true;
					} else if ($sizeOut < CHUNK_CMDDATA_MAX - 1) {
						if ($sizeDelta > 0 && $pos + $sizeIn < $totalUncomp && $sizeIn < $capGrow) {
							$sizeIn = min($sizeIn + $sizeDelta, $totalUncomp - $pos, $capGrow);
						} else {
							$end = true;
						}
					} else if ($sizeOut > CHUNK_CMDDATA_MAX) {
						$sizeIn -= $sizeDelta;
						$sizeDelta = intval($sizeDelta * 0.95);
					} else {
						$end = true;
					}
				} while (!$end);

				$payload = file_get_contents($tmp.'.'.$comp[COMP_EXT]);
				$compID  = $comp[COMP_ID]==CHUNK_RAW     ? CMP_RAW
				        : ($comp[COMP_ID]==CHUNK_RLE     ? CMP_RLE
				        : ($comp[COMP_ID]==CHUNK_PLETTER ? CMP_PLETTER
				        :                                  CMP_ZX0));
				$dataChunks[] = [$compID, $sizeIn, $payload];
				echo "    #CHUNK (".strpad($pos,5)."): sizeIn: $sizeIn bytes (out: ".strlen($payload)." bytes)\n";
				$pos += $sizeIn;
			}
			@unlink($tmp); @unlink($tmp.'.'.$comp[COMP_EXT]);
		}

		// V9938Cmd: cmdCount=1 cubriendo todo el rectángulo.
		// CLR (R#44) = primer byte real de pixel (data[0]).
		$cmdEntry  = pack("vvvvvvCCC",
			0, 0,                       // SX, SY (R32-R35)
			$dx, $dy,                   // DX, DY (R36-R39)
			$nx, $ny,                   // NX, NY (R40-R43)
			$firstPixelByte,            // CLR    (R44) — preloaded first pixel
			0,                          // ARG    (R45)
			$opcode                     // CMD    (R46)
		);
		$cmdBin = chr(CHUNK_V9938CMD).pack("vv", 1, 15).chr(1).$cmdEntry;

		// V9938CmdData chunks (suma de uncomp = N-1, complementa R#44 inicial)
		$dataBin = "";
		foreach ($dataChunks as $dc) {
			list($compID, $uncomp, $payload) = $dc;
			$dataBin .= chr(CHUNK_V9938DATA).pack("vv", 3, strlen($payload))
			          . chr($compID).pack("v", $uncomp).$payload;
		}

		return ['cmd' => $cmdBin, 'data' => $dataBin];
	}

	//=================================================================================
	function compressV9938Rectangle($file, $x, $y, $w, $h, $comp, $transparent=-1)
	{
		global $magic, $lastPalette;

		$scr = checkScreemMode($file);
		$sup = strtoupper($scr);
		echo "### Mode SCREEN ".hexdec($scr)."\n";

		// Tablas modo→unidades
		$pixelsByte  = ['5'=>2,'6'=>4,'7'=>2,'8'=>1,'A'=>1,'C'=>1];
		$bytesLine   = ['5'=>128,'6'=>128,'7'=>256,'8'=>256,'A'=>256,'C'=>256];
		$bppMode     = ['5'=>4,'6'=>2,'7'=>4,'8'=>8];

		// Validate transparency mode (PRP024 + PRP026)
		if ($transparent >= 0) {
			if ($sup == 'C') {
				die("\nERROR: --transparent-color is not supported in SC12 (pure YJK, no per-pixel A flag).\n".
				    "       Use SCREEN 10 (.SCA) instead, which supports paletted+YJK mixed mode.\n\n");
			}
			if ($sup == 'A') {
				// PRP026 — SCA transparency: paletted-visible pixels use mask=0x07 to
				// preserve dest's bits 2-0 (chroma contribution to YJK group), YJK
				// source pixels are auto-preserved (mask=0xFF, image=0x00).
				if ($transparent > 15) {
					die("\nERROR: transparent color $transparent out of range for SC10 paletted (0..15)...\n\n");
				}
				echo "### Transparent color: $transparent (SCA paletted; YJK regions auto-preserved)\n";
			} else {
				$maxColor = (1 << $bppMode[$sup]) - 1;
				if ($transparent > $maxColor) {
					die("\nERROR: transparent color $transparent out of range for SC".hexdec($sup)." (0..$maxColor)...\n\n");
				}
				echo "### Transparent color: $transparent (LMMC AND+OR streaming)\n";
			}
		}

		// Validar que x y w son múltiplos de pixelsByte (HMMC y la extracción packed lo requieren)
		if ($x % $pixelsByte[$sup] || $w % $pixelsByte[$sup]) {
			die("\nERROR: SCREEN ".hexdec($sup)." needs x and w multiple of ".$pixelsByte[$sup]."...\n\n");
		}
		echo "### Rectangle Start:($x, $y) Width:($w, $h)\n";

		// Lectura SCx (descartar header de 7 bytes)
		$in = @file_get_contents($file);
		if ($in===FALSE) { die("File not found...\n"); }
		$in = substr($in, 7);

		// Extraer el rectángulo completo (packed nativo del modo)
		$fullRect    = sliceRect($in, $x, $y, $w, $h, $pixelsByte[$sup], $bytesLine[$sup]);
		$totalUncomp = strlen($fullRect);

		if ($transparent >= 0) {
			// ---------- TRANSPARENCY PATH (LMMC AND + LMMC OR) ----------
			// LMMC trabaja por píxeles — DX/NX se pasan en píxeles directamente.
			list($maskBuf, $imageBuf) = processTransparency($fullRect, $w, $h, $sup, $transparent);
			echo "    [PASS 1: mask buffer ".strlen($maskBuf)." bytes (LMMC|AND)]\n";
			$seqMask  = buildV9938CmdSequence($maskBuf,  $x, $y, $w, $h, $comp, VDP_LMMC | LOG_AND);
			echo "    [PASS 2: image buffer ".strlen($imageBuf)." bytes (LMMC|OR)]\n";
			$seqImage = buildV9938CmdSequence($imageBuf, $x, $y, $w, $h, $comp, VDP_LMMC | LOG_OR);
			$cmdChunksBin  = $seqMask['cmd']  . $seqMask['data'];
			$cmdChunksBin .= $seqImage['cmd'] . $seqImage['data'];
			$totalReport   = strlen($maskBuf) + strlen($imageBuf);
		} else {
			// ---------- HMMC PATH (sin transparencia, legacy PRP022) ----------
			// HMMC trabaja en bytes — DX/NX se pasan convertidos a unidades de byte.
			$dxUnit = intval($x / $pixelsByte[$sup]);
			$nxUnit = intval($w / $pixelsByte[$sup]);
			$seqHmmc = buildV9938CmdSequence($fullRect, $dxUnit, $y, $nxUnit, $h, $comp, VDP_HMMC);
			$cmdChunksBin = $seqHmmc['cmd'] . $seqHmmc['data'];
			$totalReport  = $totalUncomp;
		}

		// Paleta (si paletizado)
		$palBin = "";
		if (hexdec($scr) < 8) {
			list($in2, $paper, $ink) = checkPalettedColors($in, $scr);
			$palBin = addPalette($file, $in2, $scr, NULL);
		}

		// Ensamblar fichero final con INFO al inicio
		$out  = $magic.$scr;
		$infoPos = strlen($out);
		$out .= str_repeat("\0", 15);                  // INFO placeholder

		if ($palBin && !$lastPalette) $out .= $palBin;
		$out .= $cmdChunksBin;
		if ($palBin && $lastPalette)  $out .= $palBin;

		// Contar chunks reales y rellenar el INFO
		$chunkCount = countChunks(substr($out, $infoPos+15)) + 1;   // +1 para el propio INFO
		$infoBin    = buildInfoChunk($scr, $chunkCount, $w, $h);
		$out        = substr_replace($out, $infoBin, $infoPos, 15);

		// Escribir
		echo "    In: $totalReport bytes\n    Out: ".strlen($out)." bytes [".number_format(strlen($out)/$totalReport*100,1,'.','')."%]\n";
		$fileOut = substr(basename($file), 0, -3)."IM".$scr;
		echo "### Writing $fileOut\n";
		file_put_contents($fileOut, $out);
		echo "### Done\n\n";
	}

	//=================================================================================
	function compress($tmp, $in, $pos, $sizeIn, $comp, $transparent=-1)
	{
		$data = substr($in, $pos, $sizeIn);
		$filename = $tmp.'.'.$comp[COMP_EXT];
		@unlink($filename);
		if ($comp[COMP_APP]=="raw") {
			file_put_contents($tmp.".".$comp[COMP_EXT], $data);
		} else
		if ($comp[COMP_APP]=="rle") {
			file_put_contents($tmp.".".$comp[COMP_EXT], rle_encode($data, false, NULL, true, $transparent));
		} else {
			file_put_contents($tmp, $data);
			exec($comp[COMP_APP]." $tmp", $out);
		}
		return filesize($filename);
	}

	//=================================================================================
	function rle_encode($in, $addSize=true, $mark=NULL, $eof=true, $transparent=-1)
	{
		$out = "";
		if ($addSize)
			$out = pack("v", strlen($in));

		//Find mark byte
		if ($mark===NULL) {
			$bytes = array_fill(0, 255, 0);
			for ($i=0; $i<strlen($in); $i++) {
				@$bytes[ord($in[$i])]++;
			}
			$mark = array_search(min($bytes), $bytes);
			$out .= chr($mark);
		}
		//Compress RLE
		for ($i=0; $i<strlen($in); $i++) {
			$v = $in[$i];
			$j = 0;
			while ($i+$j<strlen($in) && $in[$i+$j]==$v) $j++;
			if ($j>3 || ord($v)==$mark || ord($v)==$transparent) {
				if ($j>255) $j=255;
				if (ord($v)==$transparent) {
					$out .= chr($mark).chr(1).chr($j);	//Transparent compression
				} else {
					$out .= chr($mark).chr($j).$v;		//Normal compression
				}
				$i += $j-1;
			} else {
				$out .= $v;
			}
		}
		if ($eof) $out .= chr($mark).chr(0);
		return $out;
	}

	//=================================================================================
	function rle_encode_selection($in, $x, $y, $w, $h, $transparent=-1, $pixelsByte, $bytesLine)
	{
		$out = "";

		//Find mark byte
		$bytes = array_fill(0, 255, 0);
		for ($i=0; $i<strlen($in); $i++) {
			@$bytes[ord($in[$i])]++;
		}
		$mark = array_search(min($bytes), $bytes);
		$out = chr($mark);

		$xb = intval($x / $pixelsByte);
		$wb = intval(round($w / $pixelsByte));
		$skip = $bytesLine - $wb;

		for ($i = 0; $i < $h; $i++) {
			$out .= rle_encode(substr($in, $xb + ($y+$i)*$bytesLine, $wb), false, $mark, false, $transparent);
			if ($skip && $i<$h-1) {
				//Skip the rest of the line
				$tmpSkip = $skip;
				while ($tmpSkip) {
					$tmpSize = $tmpSkip>255 ? 255 : $tmpSkip;
					$out .= chr($mark).chr(1).chr($tmpSize);
					$tmpSkip -= $tmpSize;
				}
			}
		}
		$out .= chr($mark).chr(0);

		return $out;
	}

	//=================================================================================
	function strpad($text, $len, $character=' ')
	{
		return str_pad($text, $len, $character, STR_PAD_LEFT);
	}

?>
