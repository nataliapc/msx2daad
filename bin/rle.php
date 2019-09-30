<?php
/*
	RLE decompression for Z80
	NataliaPC

	Output format:

	0x00  2bytes  Data length ()
	0x01  1byte   Mark byte for repeat and eof
	0x02  ...     Compressed data stream
	      ...
	n-2   2bytes  Mark+$00 (EOF mark)

*/
	if ($argc!=2) {
		echo "rle.php <filein>\n\n";
		die;
	}
	if (!file_exists($argv[1])) {
		echo "file '$argv[1]' don't exists...\n\n";
		die;
	}

	$in = file_get_contents($argv[1]);
	$out = "";
	//$out .= pack("v", strlen($in));

	echo "File in:  ".strlen($in)." bytes\n";

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

	file_put_contents($argv[1].'.rle', $out);

	echo "File out: ".strlen($out)." bytes\nDone\n";
?>