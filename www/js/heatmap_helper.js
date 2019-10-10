//**********************************************************************************************************
//
//
//
//**********************************************************************************************************
let pops = [
	/*RJ*/ {lat: -22.95416, lng: -43.17444, count: 724 }, 
	/*AC*/ {lat: -9.9563, lng: -67.86543, count: 100 }, 
	/*AL*/ {lat: -9.66116, lng: -35.74146, count: 358 }, 
	/*AM*/ {lat: -3.10142, lng: -59.97418, count: 400 }, 
	/*AP*/ {lat: -0.00609, lng: -51.08485, count: 100 }, 
	/*BA*/ {lat: -13.00138, lng: -38.50749, count: 480 }, 
	/*CE*/ {lat: -3.7458, lng: -38.57565, count: 480 }, 
	///*DF2*/ {lat: -15.80416, lng: -47.88138, count: 649 }, 
	/*DF*/ {lat: -15.80416, lng: -47.88138, count: 628 }, 
	/*ES*/ {lat: -20.27694, lng: -40.30444, count: 380 }, 
	/*GO*/ {lat: -16.67642, lng: -49.24087, count: 416 }, 
	/*MA*/ {lat: -2.55583, lng: -44.30886, count: 380 }, 
	/*MG*/ {lat: -19.87, lng: -43.9647245, count: 516 }, 
	/*MI*/ {lat: 25.78236, lng: -80.19308, count: 604 },  
	/*MS*/ {lat: -20.50255, lng: -54.61166, count: 400 }, 
	/*MT*/ {lat: -15.60812, lng: -56.06511, count: 332 }, 
	/*PA*/ {lat: -1.475, lng: -48.45555, count: 480 }, 
	/*PB*/ {lat: -7.21333, lng: -35.9075, count: 100 }, 
	///*PB2*/ {lat: -7.13728, lng: -34.84511, count: 100 }, 
	/*PE*/ {lat: -8.05888, lng: -34.95333, count: 570 }, 
	/*PI*/ {lat: -5.10655, lng: -42.8113, count: 100 }, 
	/*PR*/ {lat: -25.45061, lng: -49.23211 , count: 608 }, 
	/*RN*/ {lat: -5.83961, lng: -35.20095, count: 300 }, 
	/*RO*/ {lat: -8.83444, lng: -63.94, count: 200 }, 
	/*RR*/ {lat: 2.83393, lng: -60.69473, count: 200 }, 
	/*RS*/ {lat: -30.04083, lng: -51.20666, count: 400 }, 
	/*RS2*/ {lat: -30.04083, lng: -51.20666, count: 585 }, 
	/*SC*/ {lat: -27.60138, lng: -48.51694, count: 575 }, 
	/*SE*/ {lat: -10.91455, lng: -37.05449, count: 100 }, 
	/*SP*/ {lat: -23.55613, lng: -46.72961, count: 730 }, 
	/*SP*/ {lat: -23.55613, lng: -46.72961, count: 811 }, 
	/*TO*/ {lat: -10.18388, lng: -48.3625 , count: 258 }  
]

//-----------------
//
//
//
//
function lat_lon_distance(lat1, lon1, lat2, lon2, unit) {
	if ((lat1 == lat2) && (lon1 == lon2)) {
		return 0;
	}
	else {
		var radlat1 = Math.PI * lat1/180;
		var radlat2 = Math.PI * lat2/180;
		var theta = lon1-lon2;
		var radtheta = Math.PI * theta/180;
		var dist = Math.sin(radlat1) * Math.sin(radlat2) + Math.cos(radlat1) * Math.cos(radlat2) * Math.cos(radtheta);
		if (dist > 1) {
			dist = 1;
		}
		dist = Math.acos(dist);
		dist = dist * 180/Math.PI;
		dist = dist * 60 * 1.1515;
		if (unit==undefined) unit='K';
		if (unit=="K") { dist = dist * 1.609344 }
		if (unit=="N") { dist = dist * 0.8684 }
		return dist;
	}
}

//-----------------
//
//
//
//
function lat_lon_nearest(lat, lon) {
	let near_i = -1
	let min_dist = 1000;
	let lati, loni
	let dist
	let near_dist = min_dist + 1
	for (let i = 0; i<pops.length; i++) {
		dist = lat_lon_distance(lat,lon, pops[i].lat, pops[i].lng, 'K')
		if (dist > min_dist) continue
		if (dist < near_dist) {
			near_dist = dist
			near_i = i
		}
	}
	if (near_i < 0) return [ lat, lon ]
	// console.log(near_i)
	return [ pops[near_i].lat,pops[near_i].lng ]
}


//-----------------
//
//
//
//
function v_clustering(values, k) {
	let res = []
	let n = values.length
	if (n <= k) {
		for (i = 0; i<n; i++) res.push(values[i])
		return res
	}
	
	let kcent = []
	let kidx = []
	let step =  Math.floor(n/ k)
	let i, j
	for (i = 0; i < k; i++) {
		kcent.push(values[i*step])
	}
	for (i = 0; i < n; i++) {
		let j = n/step
		if (j >= k) j = k-1
		kidx.push(j)
	}
	
	for (let m = 30; m; m--) {
		let changed = 0
		for (j = 0; j < k; j ++) {
			for (i = 0; i < n; i++) {
				if (Math.abs(values[i]-kcent[j]) < Math.abs(values[i]-kcent[kidx[i]])) {
					kidx[i] = j
					changed = 1
				}
			}
		}
		if (! changed) break
		
		for (j=0; j < k; j++) {
			let acc = 0
			let c = 0
			for (i = 0; i < n; i++) {
				if (kidx[i] == j) {
					acc += values[i]
					c ++;
				}
			}
			kcent[j] = (c>0)? acc / c: 0
		}
	}
	for (i = 0; i<n; i++) {
		res.push(kcent[kidx[i]])
	}
	return res
}

let vs = [ 100, 200, 500, 600, 700, 800, 1000]
let cs = v_clustering(vs,3)
