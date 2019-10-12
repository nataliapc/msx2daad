#!/usr/bin/php
<?php
/*
	File output format:
		Offset Size  Description
		0x0000  3    Image magic string: "IMG"
		0x0003  1    Source screen type ('5', '6', '7', '8')
		0x0004 ...   Chunks containing the image (chunk max length: 5+2048 bytes each)

	Chunk Redirect format:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type: (0:redirect)
		0x0001  2    New image location to read
		0x0002  2    Empty chunk header (filled with 0xff)

	Chunk Palette format:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type  (1:palette)
		0x0001  2    The size of the palette (32)
		0x0003  2    The size of the palette (32)
		---data---
		0x0005  32   12 bits palette data in 2 bytes format (0xRB 0x0G)

	Chunk bitmap format:
		Offset Size  Description
		--header--
		0x0000  1    Chunk type  (2:data_raw 3:data_rle 4:data_pletter)
		0x0001  2    Chunk length (max: 2048 bytes)
		0x0003  2    Uncompressed data length in bytes
		---data---
		0x0005 ...   Chunk data (1-2048 bytes length)
*/

	define('CHUNK_HEAD', 5);
	define('CHUNK_SIZE', 2043);

	define('CHUNK_REDIRECT', 0);
	define('CHUNK_PALETTE',  1);
	define('CHUNK_RAW',      2);
	define('CHUNK_RLE',      3);
	define('CHUNK_PLETTER',  4);

	$compressors = array(
		array("raw", "raw", CHUNK_RAW),
		array("rle", "rle", CHUNK_RLE),
		array("pletter", "plet5", CHUNK_PLETTER),
	);
	define('COMP_APP', 0);
	define('COMP_EXT', 1);
	define('COMP_ID',  2);

	$magic = "IMG";

	if ($argc<3) {
		showSyntax();
	}
	$cmd = strtolower($argv[1]);

	// List image content
	if ($cmd == 'l' && $argc==3) {
		$fileIn = $argv[2];
		showImageContent($fileIn);
		exit;
	}

	// Redirection
	if ($cmd == 'r' && $argc==4) {
		echo "### Creating redirection file\n";
		$fileOut = $argv[2];
		$newLoc = $argv[3];
		if (!is_numeric($newLoc)) {
			echo "ERROR: Redirect location is not a integer...\n";
			showSyntax();
		}
		echo "    Adding redirect location to $newLoc\n";
		$out = $magic.substr($fileOut,-1).chr(CHUNK_REDIRECT).pack("v",$newLoc).chr(0xff).chr(0xff);
		file_put_contents($fileOut, $out);
		echo "### Writing $fileOut\n";
		echo "### Done.\n\n";
		exit;
	}

	// Create image
	if ($cmd == 'c' && $argv>=4) {
		$fileIn = $argv[2];
		echo "### Loading $fileIn\n";
		$lines = $argv[3];
		if (!is_numeric($lines)) {
			echo "ERROR: lines must be numeric and greater than zero...\n";
			showSyntax();
		}
		$lines = intval($lines);
		$compress = "RLE";
		if ($argc>4) $compress = strtoupper($argv[4]);
		foreach ($compressors as $comp) {
			if (strtoupper($comp[0])==$compress) break;
		}
		echo "### Compressor: ".$compress."\n";

		compressChunks($fileIn, $lines, $comp);

		exit;
	}

	showSyntax();


	//=================================================================================
	function showSyntax() {
		echo "\nSyntax to list image content:\n".
			 "    compress.php l <fileIn>\n\n".
			 "Syntax to create a image IMx:\n".
			 "    compress.php c <fileIn> <lines> [compressor]\n\n".
			 "Syntax to create a location redirection:\n".
			 "    compress.php r <fileOut> <target_loc>\n\n".
			 " <fileIn>      Input file in format SCx (SC5/SC6/SC7/SC8)\n".
			 "               The palette can be inside SCx file or in PL5 PL6 PL7 files.\n".
			 " <lines>       Image lines to get from input file.\n".
			 " [compressor]  Compression type: RAW, RLE or PLETTER.\n".
			 "                 RAW: no compression but fastest load.\n".
			 "                 RLE: light compression but fast load (default).\n".
			 "                 PLETTER: high compression but slow.\n".
			 " [target_loc]  Target location number to redirect to.\n".
			 "                 ex: a 12 redirects to image 012.IMx\n".
			 "\n";
		exit(1);
	}

	//=================================================================================
	function showImageContent($fileIn) {
		global $magic;

		echo "### Reading file $fileIn\n";
		$in = file_get_contents($fileIn);

		//Magic & Screen type
		if (substr($in, 0, 3)!=$magic) {
			echo "ERROR: bad file type...\n";
			exit;
		}
		$scr = substr($in, 3, 1);
		if ($scr<'5' || $scr>'8') {
			echo "ERROR: bad screen mode ['$scr']...\n";
			exit;
		}
		echo "    Mode SCREEN $scr\n";

		// Chunks
		$pos = 4;
		$totalRaw = 0;
		$totalComp = 0;
		while ($pos < strlen($in)) {
			list($type,$sin,$sout) = array_values(unpack('cType/vSizeOut/vSizeIn', substr($in, $pos, 5)));
			switch ($type) {
				case CHUNK_REDIRECT:
					echo "    CHUNK Redirect -> ".strpad($sin, 3, '0').".IM$scr\n";
					$pos += 5;
					break;
				case CHUNK_PALETTE:
					echo "    CHUNK Palette $sout bytes\n";
					$pos += 5 + $sin;
					break;
				case CHUNK_RAW:
					echo "    CHUNK RAW Data: $sout bytes\n";
					$pos += 5 + $sin;
					break;
				case CHUNK_RLE:
					echo "    CHUNK RLE Data: $sout bytes ($sin bytes compressed) [".number_format($sin/$sout*100,1,'.','')."%]\n";
					$pos += 5 + $sin;
					break;
				case CHUNK_PLETTER:
					echo "    CHUNK PLETTER Data: $sout bytes ($sin bytes compressed) [".number_format($sin/$sout*100,1,'.','')."%]\n";
					$pos += 5 + $sin;
					break;
			}
			if ($type != CHUNK_REDIRECT) {
				$totalRaw += $sout;
				$totalComp += $sin;
			}
		}
		if ($type != CHUNK_REDIRECT) {
			echo "    Original size:   $totalRaw bytes\n";
			echo "    Compressed size: $totalComp bytes [".number_format($totalComp/$totalRaw*100,1,'.','')."%]\n";
		}
		echo "### End of file\n\n";
	}

	//=================================================================================
	function compressChunks($file, $lines, $comp) {
		global $magic;

		$tmp = "_0000000.tmp";
		$out = $magic;

		$scr = substr($file, -1);
		if ($scr<'5' || $scr>'8') {
			echo "ERROR: bad screen mode ['$scr']...\n";
			exit;
		}
		echo "### Mode SCREEN $scr\n";
		$out .= $scr;
		echo "### Lines $lines\n";

		$filePalette = substr($file, strlen($file)-3)."PL".$scr;
		if (file_exists($filePalette)) {	// Add palette chunk
			echo "### Adding image palette from file '$filePalette'\n";
			$out .= chr(CHUNK_PALETTE).pack("vv",32,32).substr(file_get_contents($filePalette), 0, 32);
		} else {
			if ($scr!=8) echo "### Palette not found\n";
		}

		$width = array(0,0,0,0,0,128,128,256,256);	// Bytes each Row in screen modes
		$in = file_get_contents($file);
		$in = substr($in, 7, $width[$scr]*$lines);
		$pos = 0;
		$i = 1;

		while ($pos < strlen($in)) {
			$sizeIn = CHUNK_SIZE;
			$sizeDelta = intval(CHUNK_SIZE/2);
			$end = false;
			do {
				$sizeOut = compress($tmp, $in, $pos, $sizeIn, $comp);
				if ($sizeIn+$pos>=strlen($in) && $sizeOut<CHUNK_SIZE) {
					$sizeIn = strlen($in)-$pos;
					$sizeOut = compress($tmp, $in, $pos, $sizeIn, $comp);
					$end = true;
				} else
				if ($sizeOut < CHUNK_SIZE-1) {
					$sizeIn += $sizeDelta;
					$sizeDelta = intval($sizeDelta*0.96);
				} else {
				if ($sizeOut > CHUNK_SIZE) {
					$sizeIn -= $sizeDelta;
					$sizeDelta = intval($sizeDelta*0.95);
				} else
					$end = true;
				}
				echo "\r\x1b[2K    #CHUNK ".strpad($i,2)." (".strpad($pos,5)."): sizeIn: $sizeIn bytes (out: $sizeOut bytes)";
			} while (!$end);
			$out .= pack("cvv", $comp[COMP_ID], $sizeOut, $sizeIn).file_get_contents($tmp.'.'.$comp[COMP_EXT]);
			echo "\n";
			$i++;
			$pos += $sizeIn;
		}

		echo "    In: ".strlen($in)." bytes\n    Out: ".strlen($out)." bytes [".number_format(strlen($out)/strlen($in)*100,1,'.','')."%]\n";
		$file = basename($file);
		$fileOut = substr($file, 0, strlen($file)-3)."IM".$scr;
		echo "### Writing $fileOut\n";
		file_put_contents($fileOut, $out);

		@unlink($tmp);
		@unlink($tmp.'.'.$comp[COMP_EXT]);
		echo "### Done\n\n";
	}

	//=================================================================================
	function compress($tmp, $in, $pos, $sizeIn, $comp) {
		$data = substr($in, $pos, $sizeIn);
		@unlink($tmp.'.'.$comp[COMP_EXT]);
		if ($comp[COMP_APP]=="raw") {
			file_put_contents($tmp.".".$comp[COMP_EXT], $data);
		} else
		if ($comp[COMP_APP]=="rle") {
			file_put_contents($tmp.".".$comp[COMP_EXT], rle_encode($data, false));
		} else {
			file_put_contents($tmp, $data);
			exec($comp[COMP_APP]." $tmp", $out);
		}
		return filesize($tmp.'.'.$comp[COMP_EXT]);
	}

	//=================================================================================
	function rle_encode($in, $addSize=true) {
		$out = "";
		if ($addSize)
			$out = pack("v", strlen($in));

		//Find mark byte
		$bytes = array_fill(0, 255, 0);
		for ($i=0; $i<strlen($in); $i++) {
			@$bytes[ord($in[$i])]++;
		}
		$mark = array_search(min($bytes), $bytes);
		$out .= chr($mark);
		//Compress RLE
		for ($i=0; $i<strlen($in); $i++) {
			$v = $in[$i];
			$j = 0;
			while ($i+$j<strlen($in) && $in[$i+$j]==$v) $j++;
			if ($j>3 || ord($v)==$mark) {
				if ($j>255) $j=255;
				$out .= chr($mark).chr($j).$v;
				$i += $j-1;
			} else {
				$out .= $v;
			}
		}
		$out .= chr($mark).chr(0);
		return $out;
	}

	//=================================================================================
	function strpad($text, $len, $character=' ') {
		return str_pad($text, $len, $character, STR_PAD_LEFT);
	}

?>