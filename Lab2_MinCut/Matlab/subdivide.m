function subdivide(name)
	img = imread(name);
	tic
	n = 16; %Any more seems to result in loss of color.
	img = round(img / n) * n;
	%figure(1);
	%imshow(img);
	%return;
	img24 = toInt24(img);
	allNodes = smash(img24);
	allEdges = cncEdges(img24);
	nodeProps = sortCount(allNodes);
	edgeProps = sortCount(allEdges);
	
	
	length(nodeProps)
	length(edgeProps)
	toc
	return;
	%toc
	%return;
	for i = 1:length(nodeProps)
		allNodes(nodeProps(i).pixelList) = randi(256^3) - 1;
	end

	
	img24 = reshape(allNodes, [size(img24, 1), size(img24, 2)]);
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
function sm = smash(img)
	 sm = reshape(img, [size(img, 1) * size(img, 2), 1]);
end
