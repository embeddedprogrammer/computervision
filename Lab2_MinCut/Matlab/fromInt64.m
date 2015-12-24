function [a, b] = fromInt64(v)
	b = mod(v, 256*256*256);
	v = (v - b) / (256*256*256);
	a = v;
end