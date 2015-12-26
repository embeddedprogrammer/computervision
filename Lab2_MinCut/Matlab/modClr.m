function modClr(a, b, c, d)
	if strcmp(a, 'hsv')
		modHsv(b, c, d);
	elseif strcmp(a, 'rgb')
		modRgb(b, c, d);
	end
end
function modHsv(h, s, v)
	imgHsv = zeros([256 256 3], 'double');
	imgHsv(:, :, 1) = getVal(h) / 256;
	imgHsv(:, :, 2) = getVal(s) / 256;
	imgHsv(:, :, 3) = getVal(v) / 256;

	% Show image
	subplot(121);
	rgb = uint8(hsv2rgb(imgHsv) * 256);
	imshow(rgb);
	addLabel(h, 'Hue');
	addLabel(s, 'Sat');
	addLabel(v, 'Val');
	
	% Show diff
	subplot(122);
	costMapHsv(imgHsv);
end
function modRgb(r, g, b)
	imgRGB = zeros([256 256 3], 'uint8');
	imgRGB(:, :, 1) = getVal(r);
	imgRGB(:, :, 2) = getVal(g);
	imgRGB(:, :, 3) = getVal(b);

	% Show image
	subplot(121);
	imshow(imgRGB);
	addLabel(r, 'Red');
	addLabel(g, 'Green');
	addLabel(b, 'Blue');
	
	% Show diff
	subplot(122);
	imgHSV = rgb2hsv(imgRGB);
	costMapHsv(imgHSV);
end
function vals = getVal(v)
	if isa(v, 'double')
		vals = v;
	elseif strcmp(v, 'x')
		vals = repmat(0:255, 256, 1); %x
	elseif strcmp(v, 'y')
		vals = repmat((0:255)', 1, 256); %y
	end	
end
function addLabel(v, label)
	if isa(v, 'double')
	elseif strcmp(v, 'x')
		xlabel(label);
	elseif strcmp(v, 'y')
		ylabel(label);
	end	
end
function costMapHsv(imgHsv)
	costImg = zeros([size(imgHsv, 1) size(imgHsv, 2)], 'uint8');
	for c=1:size(imgHsv, 2)
		for r=1:size(imgHsv, 1)
			if r < size(imgHsv, 1)
				%cst = costHsv(imgHsv(r, c, :), imgHsv(r+1, c, :));
				%costImg(r, c) = costImg(r, c) + cst;
				%costImg(r+1, c) = costImg(r+1, c) + cst;
			end
			if c < size(imgHsv, 2)
				cst = costHsv(imgHsv(r, c, :), imgHsv(r, c+1, :)); %horizontal
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r, c+1) = costImg(r, c+1) + cst;
			end
		end
	end
	imshow(costImg);
end
function c = costHsv(hsv1, hsv2)
	diff = abs(hsv1 - hsv2);
	minHsv = min(hsv1, hsv2);
	cdiff = 20*diff(1)*minHsv(2)*minHsv(3) + 2*diff(2)*minHsv(3) + diff(3);
	c = cdiff * 10000;
	%c = uint8(50 / (d + 1));
end