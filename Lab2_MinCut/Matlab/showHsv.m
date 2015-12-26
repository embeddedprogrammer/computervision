function showHsv(name)
	img = imread(name);
	%imshow(img);
	ax = get(gcf, 'children');
	set(gcf, 'WindowButtonDownFcn', @mouseCb);
	function key = mouseCb(~, callbackdata)
		pos = get(ax(1),'CurrentPoint');
		pos([1 3])
	end

	% create pixel list
	n = 8;
	img = round(img / n) * n;
	img24 = toInt24(img);
	allNodes = smash(img24);
	nodeProps = sortCount(allNodes);

	clf;
	hold on;

	n = 8;
	[x,y,z] = sphere(n);
	%hsvclr = shiftdim(rgb2hsv(shiftdim(clrs, -1)), 1);
	%hsvclr = [nodeProps.clr];
	for i = 1:length(nodeProps)
		%plot3(hsvclr(i, 1), hsvclr(i, 2), hsvclr(i, 3), '*', 'color', [clrs(i, 1) clrs(i, 2) clrs(i, 3)] * 16);
		count = nodeProps(i).count;
		sphereSize = count^(1/3) * .5;

		C = [];
		clr = toRGBSing(nodeProps(i).clr);
		%xyz = double(clr);
		xyz = rgb2hsv(toRGB(nodeProps(i).clr))*256;
		C(:,:,1) = ones(n + 1) * double(clr(1)) / 256;
		C(:,:,2) = ones(n + 1) * double(clr(2)) / 256;
		C(:,:,3) = ones(n + 1) * double(clr(3)) / 256;
		surf(xyz(1) + x*sphereSize, xyz(2) + y*sphereSize, xyz(3) + z*sphereSize, C, 'EdgeColor', clr);
	end
	grid on;
	xlabel('Hue'); ylabel('Sat'); zlabel('Val');
	%xlabel('Red'); ylabel('Green'); zlabel('Blue');

	% cm = sparse([1 1 2 2 3 3 4 5],[2 3 4 5 4 5 6 6],...
	%      [2 3 3 1 1 1 2 3],6,6)
	% [maxFlow,flowMatrix,K] = graphmaxflow(cm,1,6)
	% h = view(biograph(cm,[],'ShowWeights','on'))
	% set(h.Nodes(K(1,:)),'Color',[1 0 0])
end
function v = toInt24(img)
	v = int32(img(:,:,1))*256*256 + int32(img(:,:,2))*256 + int32(img(:,:,3));
end
function sm = smash(img)
	 sm = reshape(img, [size(img, 1) * size(img, 2), 1]);
end
function img = toRGBSing(v)
	b = mod(v, 256);
	v = (v - b) / 256;
	g = mod(v, 256);
	v = (v - g) / 256;
	r = v;
	img = [uint8(r) uint8(g) uint8(b)];
end
function img = toRGB(v)
	b = mod(v, 256);
	v = (v - b) / 256;
	g = mod(v, 256);
	v = (v - g) / 256;
	r = v;
	img = cat(3, uint8(r), uint8(g), uint8(b));
end




