% function subdivide(img)
% 	toInt24 = @(r, g, b) int32(r*256*256 + g*256 + b);
% 	toRGB = @(v) uint8([mod(floor(v/256/256), 256) mod(floor(v / 256), 256) mod(v, 256)]);
% 	img(:, :, 1), img(:, :, 1), img(:, :, 1)
% end
function toImg24(img)
	img24 = toInt24(img);
	imgReproduced = toRGB(img24);
	size(imgReproduced)
	imshow(imgReproduced);
	set(gcf, 'WindowButtonDownFcn', @mouseCb);
	ax = get(gcf, 'children');	
	function mouseCb(~, callbackdata)
		pos = get(ax(1),'CurrentPoint');
		pos = int32(pos([1 3]));
		img(pos(2), pos(1), :)
		imgReproduced(pos(2), pos(1), :)
	end	
end
function v = toInt24(img)
	v = int32(img(:,:,1))*256*256 + int32(img(:,:,2))*256 + int32(img(:,:,3));
end
function img = toRGB(v)
	b = mod(v, 256);
	v = (v - b) / 256;
	g = mod(v, 256);
	v = (v - g) / 256;
	r = v;
	img = cat(3, uint8(r), uint8(g), uint8(b));
end
