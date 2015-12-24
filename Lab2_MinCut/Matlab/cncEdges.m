function cncClrs = cncEdges(img)
	 cncClrs = [clrToInt64(smash(img(:, 1:size(img,2)-1)), smash(img(:, 2:size(img,2))))
				clrToInt64(smash(img(1:size(img,1)-1, :)), smash(img(2:size(img,1), :)))
				clrToInt64(smash(img(:, 2:size(img,2))), smash(img(:, 1:size(img,2)-1)))
				clrToInt64(smash(img(2:size(img,1), :)), smash(img(1:size(img,1)-1, :)))];
end
function sm = smash(img)
	 sm = reshape(img, [size(img, 1) * size(img, 2), 1]);
end
function v = clrToInt64(a, b)
	v = int64(a)*256*256*256 + int64(b);
end
