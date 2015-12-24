function costMap(name)
	tic
	img = imread(name);
	costImg = zeros([size(img, 1) size(img, 2)], 'uint8');
	for c=1:size(img, 2)
		for r=1:size(img, 1)
			if r < size(img, 1)
				cst = cost(img(r, c, :), img(r+1, c, :));
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r+1, c) = costImg(r+1, c) + cst;
			end
			if c < size(img, 2)
				cst = cost(img(r, c, :), img(r, c+1, :));
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r, c+1) = costImg(r, c+1) + cst;
			end
		end
	end
	imshow(costImg);
	toc;
end
function d = clrDiff(v1, v2)
	d = sum(abs(int16(v1 - v2)));
end
function c = cost(v1, v2)
	d = clrDiff(v1, v2);
% 	if d < 5
% 		c = 100;
% 	else
% 		c = 0;
% 	end
	c = uint8(50 / (d + 1));
end