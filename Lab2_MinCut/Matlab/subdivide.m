function subdivide(name)
	img = imread(name);
	tic
	n = 16; %Any more seems to result in loss of color.
	img = round(img / n) * n;
	%figure(1);
	%imshow(img);
	%return;
	img24 = toInt24(img);
	allClrs = reshape(img24, [size(img24, 1) * size(img24, 2), 1]);
	
% 	clrs = sortrows(allClrs, 1);
% 
% 	size(allClrs)
% 	size(clrs)
% 	for i = 1:length(clrs)
% 		allClrs(allClrs == clrs(i)) = randi(256^3) - 1;
% 	end

	props = sortCount(allClrs);
	%length(props)
	%toc
	%return;
	for i = 1:length(props)
		allClrs(props(i).pixelList) = randi(256^3) - 1;
	end

	
	img24 = reshape(allClrs, [size(img24, 1), size(img24, 2)]);
	imgReproduced = toRGB(img24);
	figure(2);
	imshow(imgReproduced);
	set(gcf, 'WindowButtonDownFcn', @mouseCb);
	ax = get(gcf, 'children');
	function mouseCb(~, callbackdata)
		pos = get(ax(1),'CurrentPoint');
		pos = int32(pos([1 3]));
		img(pos(2), pos(1), :)
		imgReproduced(pos(2), pos(1), :)
	end
	toc
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
