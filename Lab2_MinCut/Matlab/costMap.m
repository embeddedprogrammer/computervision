function costMap(name)
	tic
	img = imread(name);
	imgHsv = rgb2hsv(img);
	costImg = zeros([size(img, 1) size(img, 2)], 'uint8');
	for c=1:size(img, 2)
		for r=1:size(img, 1)
			if r < size(img, 1)
				%cst = cost(img(r, c, :), img(r+1, c, :));
				cst = costHsv(imgHsv(r, c, :), imgHsv(r+1, c, :));
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r+1, c) = costImg(r+1, c) + cst;
			end
			if c < size(img, 2)
				%cst = cost(img(r, c, :), img(r, c+1, :));
				cst = costHsv(imgHsv(r, c, :), imgHsv(r, c+1, :));
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r, c+1) = costImg(r, c+1) + cst;
			end
		end
	end
	imshow(costImg);
	toc;
end
function c = cost(v1, v2)
	d = sum(abs(int16(v1 - v2)));
	c = uint8(50 / (d + 1));
end
function c = costHsv(hsv1, hsv2)
	% (s*v + s*v)*deltaH + (v + v)*deltaS + deltaV
	diff = abs(hsv1 - hsv2);
	cdiff = (hsv1(2)*hsv1(3)+hsv2(2)*hsv2(3))*5*diff(1) + (hsv1(3)+hsv2(3))*diff(2) + diff(3);
	c = cdiff * 100;
	%c = uint8(50 / (d + 1));
end