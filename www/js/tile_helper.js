;
//-----------------
//
//
//
//
function tile2long(x,z) {
  return (x/Math.pow(2,z)*360-180);
}

//-----------------
//
//
//
//
function tile2lat(y,z) {
	var n=Math.PI-2*Math.PI*y/Math.pow(2,z);
	return (180/Math.PI*Math.atan(0.5*(Math.exp(n)-Math.exp(-n))));
}

//-----------------
//
//
//
//
function tile2longX(x,z) {
	let c1 = tile2long(x,z)
	let c2 = tile2long(x+1,z)
	return (c1+c2)/2

}

//-----------------
//
//
//
//
function tile2latX(x,z) {
	let c1 = tile2lat(x,z)
	let c2 = tile2lat(x+1,z)
	return (c1+c2)/2
}

