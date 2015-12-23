function cncClrs = cncEdges(img)
	 cncClrs = [[smash(img(:, 1:size(img,2)-1)) smash(img(:, 2:size(img,2)))]
				[smash(img(1:size(img,1)-1, :)) smash(img(2:size(img,1), :))]];
end
function sm = smash(img)
	 sm = reshape(img, [size(img, 1) * size(img, 2), 1]);
end
