#!/usr/bin/php
<?php
/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	========================================================

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

	Chunk V9990 BitBlt:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type  (32)
		0x0001  2    Chunk data length (always 21 bytes)
		0x0003  2    Empty chunk header (0x0000)
		---data---
		0x0005 ...   RAW VDP Command data (21 bytes length)

	Chunk V9990 Command Data RAW:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type  (33)
		0x0001  2    Chunk data length (max: 2043 bytes)
		0x0003  2    Uncompressed data length in bytes
		---data---
		0x0005 ...   Image data (1-2043 bytes length)

*/
	//=================================================================================
	interface InterfaceGRB {
		function toRaw($rgb24) : String;
		function getBPP() : Int;
	}

	class GRB8 implements InterfaceGRB {
		function toRaw($rgb24) : String {
			$r = round((($rgb24 >> 16) & 0xff) * 7 / 255);
			$g = round((($rgb24 >> 8) & 0xff) * 7 / 255);
			$b = round(($rgb24 & 0xff) * 3 / 255);
			$grb = ($g << 5) | ($r << 2) | $b;
			return pack("C", $grb & 0xff);
		}
		function getBPP() : Int { return 8; }
	}

	class GRB16 implements InterfaceGRB {
		function toRaw($rgb24) : String {
			$r = round((($rgb24 >> 16) & 0xff) * 31 / 255);
			$g = round((($rgb24 >> 8) & 0xff) * 31 / 255);
			$b = round(($rgb24 & 0xff) * 31 / 255);
			$grb = ($g << 10) | ($r << 5) | $b;
			return pack("v", $grb);
		}
		function getBPP() : Int { return 16; }
	}


	define('CHUNK_HEAD', 5);
	define('CHUNK_SIZE', 2043);

	define('CHUNK_REDIRECT', 0);
	define('CHUNK_PALETTE',  1);
	define('CHUNK_RAW',      2);
	define('CHUNK_RLE',      3);
	define('CHUNK_PLETTER',  4);
	define('CHUNK_RESET',   16);
	define('CHUNK_CLS',     17);
	define('CHUNK_SKIP',    18);
	define('CHUNK_PAUSE',   19);
	define('CHUNK_G9K_CMD', 32);
	define('CHUNK_G9K_RAW', 33);
	define('CHUNK_G9K_RLE', 34);
	define('CHUNK_G9K_PLETTER', 35);

	$compressors = array(
		array("raw", "raw", CHUNK_RAW, "RAW", CHUNK_G9K_RAW),
		array("rle", "rle", CHUNK_RLE, "RLE", CHUNK_G9K_RLE),
		array("pletter", "plet5", CHUNK_PLETTER, "PLETTER", CHUNK_G9K_PLETTER),
	);
	define('RAW', 0);
	define('RLE', 1);
	define('PLETTER', 2);
	define('COMP_APP',  0);
	define('COMP_EXT',  1);
	define('COMP_ID',   2);
	define('COMP_NAME', 3);
	define('COMP_ID_G9K',   4);

	if (!extension_loaded('gd')) {
		die("\nERROR: The PHP \"gd/gd2\" extension must be installed...\n\n");
	}

	$appname = basename($argv[0]);
	$magic = "IMG";
	$lastPalette = false;

	if (strtoupper(substr(PHP_OS, 0, 3)) === 'WIN') {
		$compressors[PLETTER][COMP_APP] = "pletter.exe";
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
		compressRectangle($fileIn, $x, $y, $w, $h, $transparent);
		exit;
	}

	if ($cmd == 'gs' && ($argc==3 || $argc==3+6)) {
		$fileIn = $argv[2];
		echo "### Loading $fileIn\n";
		if (!is_numeric($argv[3]) || !is_numeric($argv[4]) || !is_numeric($argv[5]) || !is_numeric($argv[6]) || !is_numeric($argv[7]) || !is_numeric($argv[8])) {
			echo "ERROR: sx, sy, dx, dy, nx, ny must be numeric and greater or equal to zero...\n";
			exit;
		}
		//Coords
		$sx = intval($argv[3]);
		$sy = intval($argv[4]);
		$dx = intval($argv[5]);
		$dy = intval($argv[6]);
		$nx = intval($argv[7]);
		$ny = intval($argv[8]);
		G9k_createRectangle($fileIn, $sx, $sy, $dx, $dy, $nx, $ny);
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
		if ($transparent>=0) {
			echo "### Transparent color: $transparent\n";
			$comp = $compressors[1];
			$compress = $comp[COMP_NAME]." (forced)";
		}
		echo "### Compressor: $compress\n";
		//Compress chunks
		compressChunks($fileIn, $lines, $comp, $transparent, NULL, NULL);
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
			 "IMGWIZARD v1.3.01 for MSX2DAAD\n".
			 "===================================================================\n".
			 "A tool to create and manage MSX image files in several screen modes\n".
			 "to be used by MSX2DAAD engine.\n\n".
			 "L) List image chunks:\n".
			 "    $appname l <fileIn.IM?>\n\n".
			 "C) Create an image IMx (CL - Create the palette at last chunk):\n".
			 "    $appname c[l] <fileIn.SC?> <lines> [compressor | transparent_color]\n\n".
			 "S) Create an image from a rectangle:\n".
			 "    $appname s <fileIn.SC?> <x> <y> <w> <h> [transparent_color]\n\n".
			 "GS) Create a V9990 image from a rectangle:\n".
			 "    $appname gs <fileIn.PNG> [<sx> <sy> <dx> <dy> <nx> <ny>]\n\n".
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
			 " [compressor]  Compression type: RAW, RLE or PLETTER.\n".
			 "                 RAW: no compression but fastest load.\n".
			 "                 RLE: light compression but fast load (default).\n".
			 "                 PLETTER: high compression but slow.\n".
			 " [transparent] Optional: the color index that will become transparent (decimal).\n".
			 "               Compression is forced to RLE.\n".
			 " <target_loc>  Target location number to redirect to.\n".
			 "                 ex: a 12 redirects to image 012.IMx\n".
			 "\n".
			 "Example: $appname c image.sc8 96 rle\n".
			 "\n";
		exit(1);
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
		if (!file_exists($fileIn)) {
			die("ERROR: file not exists...\n");
		}
		$in = file_get_contents($fileIn);

		//Magic & Screen type
		if (substr($in, 0, 3)!=$magic) {
			echo "ERROR: bad file type...\n";
			exit;
		}
		$out = substr($in, 0, 4);

		//Screen mode
		$scr = strtoupper(substr($in, 3, 1));
		if (($scr<'5' || $scr>'8') && $scr!='A' && $scr!='C' && $scr!='G') {
			echo "ERROR: bad screen mode ['$scr']...\n";
			exit;
		}
		if ($scr == 'G') {
			$scrTxt = "V9990 Bitmap";
		} else {
			$scrTxt = hexdec($scr);
		}
		echo "### Mode SCREEN ".$scrTxt."\n";

		// Chunks
		$pos = 4;
		$totalRaw = 0;
		$totalComp = 0;
		$id = 1;
		while ($pos < strlen($in)) {
			list($type,$sin,$sout) = array_values(unpack('cType/vSizeOut/vSizeIn', substr($in, $pos, 5)));
			$data = substr($in, $pos + 5);
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
				case CHUNK_G9K_CMD:
					list($sx,$sy,$dx,$dy,$nx,$ny,$arg,$log,$msk,$fc,$bc,$cmd) = 
						array_values(unpack("vsx/vsy/vdx/vdy/vnx/vny/Carg/Clog/vmsk/vfc/vbc/Ccmd", $data));
					echo "    CHUNK $id: G9K_CMD Data: $sout bytes\n";
					echo "             sx:$sx sy:$sy dx:$dx dy:$dy nx:$nx ny:$ny arg:$arg log:$log mask:$msk fc:$fc bc:$bc cmd:$cmd\n";
					$size += $sin;
					break;
				case CHUNK_G9K_RAW:
					echo "    CHUNK $id: G9K_RAW Data: $sout bytes\n";
					$size += $sin;
					break;
				case CHUNK_G9K_RLE:
					echo "    CHUNK $id: G9K_RLE Data: $sout bytes\n";
					$size += $sin;
					break;
				case CHUNK_G9K_PLETTER:
					echo "    CHUNK $id: G9K_PLETTER Data: $sout bytes\n";
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
				default:
					echo "    CHUNK $id: UNKNOWN CHUNK TYPE!!!! [**Aborted**]\n\n";
					exit;
			}
			if ($type != CHUNK_REDIRECT) {
				$totalRaw += $sout;
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
	function G9k_createRectangle($fileName, $sx, $sy, $dx, $dy, $nx, $ny, $bpp=16, $compressor=NULL)
	{
		define("CHUNKMAX", 2043);
		global $magic;
		global $compressors;
		$tmp = tempnam(sys_get_temp_dir(), 'imgwiz');
		$rgb = new GRB8();
		$outRawSize = $nx * $ny * ($rgb->getBPP()/8) + 21;
		$id = 1;

		// RGB Data
		$png = @imageCreateFromPng($fileName);
		if ($png == null) {
			die("\nERROR: Error loading PNG image...\n\n");
		}
		if (!imageIsTrueColor($png)) {
			$png = @imagePaletteToTrueColor($png);
		}
		$bmpData = "";
		for ($y = 0; $y < $ny; $y++) {
			for ($x = 0; $x < $nx; $x++) {
				$rgb24 = imagecolorat($png, $sx+$x, $sy+$y);
				$bmpData .= $rgb->toRaw($rgb24);
			}
		}

		if ($compressor==NULL) {
			$compressor = $compressors[PLETTER];
		}

		// LMMC Command
		$out = $magic."G";
		$cmdData = pack("vvvvvvCCvvvC", 0, 0, $dx, $dy, $nx, $ny, 0, 12, 0xffff, 0, 0, 0x10);
		$out .= pack("cvv", CHUNK_G9K_CMD, 21, 21).$cmdData;
		echo "    #CHUNK ".strpad($id,2)." G9K_CMD size: ".strlen($cmdData)." bytes\n";

		while (($size = strlen($bmpData)) > 0) {
			$id++;

			// RAW
			if ($compressor==$compressors[RAW]) {
				if ($size > 2043) {
					$tmpSize = 2043;
				} else {
					$tmpSize = $size;
				}
//				$size -= $tmpSize;
				$tmpData = substr($bmpData, 0, $tmpSize);
				$outSize = $tmpSize;
			} else
			// RLE
			if ($compressor==$compressors[RLE]) {
				$tmpSize = 2040;
				if ($tmpSize > $size) { $tmpSize = $size; }
				do {
					$tmpData = rle_encode(substr($bmpData, 0, $tmpSize), false, NULL, true);
					if (strlen($tmpData) <= 2043 && $tmpSize!=$size) {
						$tmpSize += 10;
					}
					echo "\r\x1b[2K    #CHUNK ".strpad($id,2)." sizeIn: $tmpSize bytes (out: ".strlen($tmpData)." bytes)";
				} while(strlen($tmpData) <= 2043 && $tmpSize<$size);
				while(strlen($tmpData) > 2043 || $tmpSize>$size) {
					$tmpSize--;
					$tmpData = rle_encode(substr($bmpData, 0, $tmpSize), false, NULL, true);
					echo "\r\x1b[2K    #CHUNK ".strpad($id,2)." sizeIn: $tmpSize bytes (out: ".strlen($tmpData)." bytes)";
				}
				$outSize = strlen($tmpData);
			} else
			// EXTERNAL COMPRESSORS (PLETTER, etc...)
			{
				$tmpSize = CHUNKMAX;
				$outSize = $oldOutSize = 0;
				if ($tmpSize > $size) { $tmpSize = $size; }
				do {
					$oldOutSize = $outSize;
					$outSize = compress($tmp, $bmpData, 0, $tmpSize, $compressor);
					$tmpData = file_get_contents($tmp.'.'.$compressor[COMP_EXT]);
					if ($outSize <= CHUNKMAX && $tmpSize!=$size) {
						if ($outSize == $oldOutSize) {
							$tmpSize += 500;
						} else {
							$tmpSize += 10;
						}
					}
					echo "\r\x1b[2K    #CHUNK ".strpad($id,2)." sizeIn: $tmpSize bytes (out: $outSize bytes)";
				} while($outSize <= CHUNKMAX && $tmpSize<$size);
				while($outSize > CHUNKMAX || $tmpSize>$size) {
					if ($tmpSize > $size) {
						$tmpSize = $size;
					} else {
						$tmpSize -= 10;
					}
					$outSize = compress($tmp, $bmpData, 0, $tmpSize, $compressor);
					if ($outSize < CHUNKMAX-3 && $tmpSize!=$size) {
						$tmpSize += 9;
						$outSize = compress($tmp, $bmpData, 0, $tmpSize, $compressor);
					}
					$tmpData = file_get_contents($tmp.'.'.$compressor[COMP_EXT]);
					echo "\r\x1b[2K    #CHUNK ".strpad($id,2)." sizeIn: $tmpSize bytes (out: $outSize bytes)";
				}
			}

			$bmpData = substr($bmpData, $tmpSize);
			$out .= pack("Cvv", $compressor[COMP_ID_G9K], $outSize, $tmpSize).$tmpData;
			echo "\r\x1b[2K    #CHUNK ".strpad($id,2)." G9K_".$compressor[COMP_NAME]." sizeIn: $tmpSize  (out: ".strlen($tmpData)." bytes)\n";
		}

		// Show result
		echo "    In: ".$outRawSize." bytes\n    Out: ".strlen($out)." bytes [".number_format(strlen($out)/$outRawSize*100,1,'.','')."%]\n";
		$fileBaseName = basename($fileName);
		$fileOut = substr(basename($fileBaseName), 0, strlen($fileBaseName)-3)."IMG";

		// Write put file
		echo "### Writing $fileOut\n";
		file_put_contents($fileOut, $out);
		echo "### Done\n\n";
	}

	//=================================================================================
	function compressRectangle($file, $x, $y, $w, $h, $transparent=-1, $in=NULL, $pal=NULL)
	{
		global $magic;
		$id = 1;

		// Check screen mode
		$out = $magic;
		$scr = checkScreemMode($file);
		echo "### Mode SCREEN ".hexdec($scr)."\n";
		if (hexdec($scr)>=10 && ($x%4!=0 || $w%4!=0)) {
			die("\nERROR: SCREEN 10/12 needs 'x' and 'w' input to be multiple of 4...\n\n");
		}
		$out .= $scr;
		echo "### Rectangle Start:($x, $y) Width:($w, $h)\n";

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
		$id = 1;
		$tmp = tempnam(sys_get_temp_dir(), 'imgwiz');

		// Bytes each Row in screen modes
		$width = array(0,0,0,0,0,128,128,256,256,'A'=>256,'C'=>256);
		
		// Check screen mode
		$out = $magic;
		$scr = checkScreemMode($file);
		echo "### Mode SCREEN ".hexdec($scr)."\n";
		$out .= $scr;
		echo "### Lines $lines\n";

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
					$out .= chr($mark).chr(2).chr($j);	//Transparent compression
				} else {
					if (ord($v)==$mark && $j<=3) $j=1;
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
