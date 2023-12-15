// a couple helper functions for converting to/from arbitrary 64bit values to hexadecimal encoding

const href = "example.svg?1ac";// document.defaultView.location.href;
const q_pos = href.indexOf("?") + 1;
const svgNS = "http://www.w3.org/2000/svg";
const script = document.getElementById("S")!;	// the id definitely exists

const x = new Int8Array([
	 0,-2,-1,-3,
	 2, 3, 1, 2,
	-2,-1, 0, 1
]);
const y = new Int8Array([
	 4, 4, 1, 1,
	-2, 1, 1, 4,
	-2,-5,-2,-5
]);

if(q_pos)	// if the query exists, it should only include the hex data
{
	const hex = href.substring(q_pos);	// hexadecimal encoded string of bit states
	const bin = BigInt("0x" + hex);	//convert to binary
	for(var i = 0; i < 12; ++i)
	{
		const bit = document.createElementNS(svgNS, "use");
		const bool = Boolean((bin >> BigInt(i)) & 1n);
		bit.setAttribute("href", '#'+(+bool));
		if (x[i])	// only set if it's non-zero
			bit.setAttribute("x", x[i].toString());
		bit.setAttribute("y", y[i].toString());	// y is never non-zero

		if(bool)
			script.after(bit);
		else
			script.before(bit);
	}
}

//NOTE: remember to compile the Web Assembly portion with -s WASM_BIGINT

// a "number" is actually a Float64 value but it's the only natively supported 64-bit type in JS
//  this means that a simple return from WASM doesn't work if the result doesn't fit in 53-bits
//  of data because I64 could only be converted to Float64, however using a shim that converts
//  via a union to a double we can still return a 64-bit value as a single return without having
//  to deal with memory shenanigans to return 2*32-bit values so long as we don't need to perform
//  integer math on them, which we don't
function I64toHex(raw: number)
{
	var buf = new ArrayBuffer(8);	// JS numbers are Float64, ie 8 bytes
	var f64 = new Float64Array(buf);
	f64[0] = raw;
	var ui64 = new BigUint64Array(buf);
	return ui64[0].toString(16).padStart(15, '0');	// pad length needs to be dependent on nDiv3 value
}