classdef clr
methods (Static)
%CLASS BEGIN

% Collapse and expand colors
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
function v = clrToInt64(a, b)
	v = int64(a)*256*256*256 + int64(b);
end
function [a, b] = fromInt64(v)
	b = mod(v, 256*256*256);
	v = (v - b) / (256*256*256);
	a = v;
end

% SortCount and Edge properties
function props = sortCount(allClrs)
	allClrsI(:, 1) = allClrs;
	allClrsI(:, 2) = 1:length(allClrs);
	
	% Create pixel list for each color.
	% (unique only gives us one index, not all indices)
	clrs = sortrows(allClrsI, 1);
	startI = 1;
	props = [];
	num = 1;
	for i = 1:size(clrs, 1)
		if i == size(clrs, 1) || clrs(i, 1) ~= clrs(i + 1, 1)
			endI = i;
			clr = clrs(startI, 1);
			pixelList = clrs(startI:endI, 2);
			count = endI - startI + 1;
			s = struct('clr', clr, 'pixelList', pixelList, 'count', count);
			if isempty(props)
				props = s;
			else
				props(num) = s;
			end
			num = num + 1;
			startI = i + 1;
		end
	end
end
function cncClrs = cncEdges(img)
	 cncClrs = [clr.clrToInt64(clr.smash(img(:, 1:size(img,2)-1)), clr.smash(img(:, 2:size(img,2))))
				clr.clrToInt64(clr.smash(img(1:size(img,1)-1, :)), clr.smash(img(2:size(img,1), :)))
				clr.clrToInt64(clr.smash(img(:, 2:size(img,2))), clr.smash(img(:, 1:size(img,2)-1)))
				clr.clrToInt64(clr.smash(img(2:size(img,1), :)), clr.smash(img(1:size(img,1)-1, :)))];
end
function costMap = calcEdgeNodeProps(nodeProps, edgeProps)
	nodeMap = containers.Map([nodeProps.clr], [nodeProps.count]);
	cst = zeros(size(edgeProps), 'uint8');
	for i=1:length(edgeProps)
		edgeProp = edgeProps(i);
		[a, b] = clr.fromInt64(edgeProp.clr);
		aCnt = nodeMap(a);
		bCnt = nodeMap(b);
		cCnt = edgeProp.count;
		cst(i) = 20*cCnt^2/(aCnt*bCnt);
	end
	costMap = containers.Map([edgeProps.clr], cst);
end

%COST FUNCTIONS
function c = costHsv(hsv1, hsv2)
	diff = abs(hsv1 - hsv2);
	minHsv = min(hsv1, hsv2);
	cdiff = 20*diff(1)*minHsv(2)*minHsv(3) + 2*diff(2)*minHsv(3) + diff(3);
	c = cdiff * 100;
	%c = uint8(50 / (d + 1));
end
function cst = cost(v1, v2, nodeProps, edgeProps)
	a = nodeProps([nodeProps.clr] == v1).count;
	b = nodeProps([nodeProps.clr] == v2).count;
	c = edgeProps([edgeProps.clr] == clrToInt64(v1, v2)).count;
	cst = uint8(100*c^2/(a*b));
end
function c = costOld(v1, v2)
	d = clrDiff(v1, v2);
% 	if d < 5
% 		c = 100;
% 	else
% 		c = 0;
% 	end
	c = uint8(50 / (d + 1));
end
function d = clrDiff(v1, v2)
	d = sum(abs(int16(v1 - v2)));
end
function c = costZ(v1, v2)
	d = sum(abs(int16(v1 - v2)));
	c = uint8(50 / (d + 1));
end
function c = costX(d)
	c = uint8(255 ./ (d + 1));
% 	u = 3;
% 	B = 3;
% 	x = double(d);
% 	c = uint8((atan(-(x - u)/(B/4))/pi+.5)*256);
end

%Testing functions
function testConvN
	tic
	A = rand(256, 256, 256);
	B = rand(3, 3, 3);
	A = convn(A, B, 'same');
	A = convn(A, B, 'same');
	A = convn(A, B, 'same');
	toc
end

function mapTest
	%mapObj = containers.Map('KeyType', 'int64', 'ValueType', 'int32');
	tic
	vals = randi(500, [50000 1]);
	props = clr.sortCount(vals);

	% mapObj = containers.Map([props.clr], [props.count]);
	% for i = 1:length(vals)
	% 	count = mapObj(vals(i));
	% end

	% for i = 1:length(vals)
	% 	count = props([props.clr] == vals(i)).count;
	% end

	% for i = 1:length(vals)
	% 	count = props(vals(i)).count;
	% end
	count = [props(vals).count];

	toc
	tic
	zz = zeros([256 256 256]);
	toc
end

% Top level
function costMap(name)
	tic
	img = imread(['Pics/' name]);
	imgHsv = rgb2hsv(img);
	costImg = zeros([size(img, 1) size(img, 2)], 'uint8');
	for c=1:size(img, 2)
		for r=1:size(img, 1)
			if r < size(img, 1)
				%cst = cost(img(r, c, :), img(r+1, c, :));
				cst = clr.costHsv(imgHsv(r, c, :), imgHsv(r+1, c, :));
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r+1, c) = costImg(r+1, c) + cst;
			end
			if c < size(img, 2)
				%cst = cost(img(r, c, :), img(r, c+1, :));
				cst = clr.costHsv(imgHsv(r, c, :), imgHsv(r, c+1, :));
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r, c+1) = costImg(r, c+1) + cst;
			end
		end
	end
	imshow(costImg);
	toc;
end
function modClr(a, b, c, d)
	if strcmp(a, 'hsv')
		clr.modHsv(b, c, d);
	elseif strcmp(a, 'rgb')
		clr.modRgb(b, c, d);
	end
end
function modHsv(h, s, v)
	imgHsv = zeros([256 256 3], 'double');
	imgHsv(:, :, 1) = clr.getVal(h) / 256;
	imgHsv(:, :, 2) = clr.getVal(s) / 256;
	imgHsv(:, :, 3) = clr.getVal(v) / 256;

	% Show image
	subplot(121);
	rgb = uint8(hsv2rgb(imgHsv) * 256);
	imshow(rgb);
	clr.addLabel(h, 'Hue');
	clr.addLabel(s, 'Sat');
	clr.addLabel(v, 'Val');
	
	% Show diff
	subplot(122);
	clr.costMapHsv(imgHsv);
end
function modRgb(r, g, b)
	imgRGB = zeros([256 256 3], 'uint8');
	imgRGB(:, :, 1) = clr.getVal(r);
	imgRGB(:, :, 2) = clr.getVal(g);
	imgRGB(:, :, 3) = clr.getVal(b);

	% Show image
	subplot(121);
	imshow(imgRGB);
	clr.addLabel(r, 'Red');
	clr.addLabel(g, 'Green');
	clr.addLabel(b, 'Blue');
	
	% Show diff
	subplot(122);
	imgHSV = rgb2hsv(imgRGB);
	clr.costMapHsv(imgHSV);
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
				%cst = clr.costHsv(imgHsv(r, c, :), imgHsv(r+1, c, :));
				%costImg(r, c) = costImg(r, c) + cst;
				%costImg(r+1, c) = costImg(r+1, c) + cst;
			end
			if c < size(imgHsv, 2)
				cst = clr.costHsv(imgHsv(r, c, :), imgHsv(r, c+1, :))*100; %horizontal
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r, c+1) = costImg(r, c+1) + cst;
			end
		end
	end
	imshow(costImg);
end

%Kernel functions
function plotCost()
	d = 1:512;
	c = cost(d);
	plot(d, c);
end
function kernel(name)
	img = imread(['Pics/' name]);
	subplot(221); imshow(img);
	subplot(222); clr.applyKernel(img, [1 -1]);
	subplot(223); clr.applyKernel(img, [1 -1; 1 -1; 1 -1; 1 -1] / 4);
	subplot(224); clr.medFiltKernel(img, [3 1]);
	set(gcf, 'WindowButtonDownFcn', @mouseCb);
	%sbk = subplot(221);
	ax = get(gcf, 'children');	
	function mouseCb(~, callbackdata)
		%pos = get(sbk,'CurrentPoint');
		pos = get(ax,'CurrentPoint');
		pos = int32(pos([1 3]));
		if pos(1) >= 1 && pos(1) <= size(img, 2) && pos(2) >= 1 && pos(2) <= size(img, 1)
			rgb = img(pos(2), pos(1), :);
			hsv = floor(rgb2hsv(rgb) * 256);
			fprintf('rgb: %d %d %d ', rgb(1,1,1), rgb(1,1,2), rgb(1,1,3));
			fprintf('hsv: %d %d %d\n', hsv(1,1,1), hsv(1,1,2), hsv(1,1,3));
		end
	end	
end
function medFiltKernel(img, rc)
	imgMed(:,:,1) = medfilt2(img(:,:,1), rc);
	imgMed(:,:,2) = medfilt2(img(:,:,2), rc);
	imgMed(:,:,3) = medfilt2(img(:,:,3), rc);
	clr.applyKernel(imgMed, [1 -1]);
end
function applyKernel(img, k)
	imgFloat = double(img);
	imgConv(:,:,1) = conv2(imgFloat(:,:,1), k);
	imgConv(:,:,2) = conv2(imgFloat(:,:,2), k);
	imgConv(:,:,3) = conv2(imgFloat(:,:,3), k);
	imshow(clr.costX(rgb2gray(uint8(abs(imgConv)))));
end
function newCostMap(name)
	tic
	img = imread(['Pics/' name]);
	n = 16; %Any more seems to result in loss of color.
	img = round(img / n) * n;
	%figure(1);
	%imshow(img);
	%return;
	img24 = clr.toInt24(img);
	allNodes = clr.smash(img24);
	allEdges = clr.cncEdges(img24);
	nodeProps = clr.sortCount(allNodes);
	edgeProps = clr.sortCount(allEdges);
	costMap = clr.calcEdgeNodeProps(nodeProps, edgeProps);
	
	costImg = zeros([size(img, 1) size(img, 2)], 'uint8');
	for c=1:size(img, 2)
		for r=1:size(img, 1)
			if r < size(img, 1)
				%cst = cost(img24(r, c), img24(r+1, c), nodeProps, edgeProps);
				cst = costMap(clr.clrToInt64(img24(r, c), img24(r+1, c)));
				%cst = costOld(img(r, c, :), img(r+1, c, :));
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r+1, c) = costImg(r+1, c) + cst;
			end
			if c < size(img, 2)
				%cst = cost(img24(r, c), img24(r, c+1), nodeProps, edgeProps);
				cst = costMap(clr.clrToInt64(img24(r, c), img24(r, c+1)));
				%cst = costOld(img(r, c, :), img(r, c+1, :));
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r, c+1) = costImg(r, c+1) + cst;
			end
		end
	end
	imshow(costImg);
	toc;
end
function showHsv(name)
	img = imread(['Pics/' name]);
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
	img24 = clr.toInt24(img);
	allNodes = clr.smash(img24);
	nodeProps = clr.sortCount(allNodes);

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
		cl = clr.toRGBSing(nodeProps(i).clr);
		%xyz = double(clr);
		xyz = rgb2hsv(clr.toRGB(nodeProps(i).clr))*256;
		C(:,:,1) = ones(n + 1) * double(cl(1)) / 256;
		C(:,:,2) = ones(n + 1) * double(cl(2)) / 256;
		C(:,:,3) = ones(n + 1) * double(cl(3)) / 256;
		surf(xyz(1) + x*sphereSize, xyz(2) + y*sphereSize, xyz(3) + z*sphereSize, C, 'EdgeColor', cl);
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
function subdivide(name)
	img = imread(['Pics/' name]);
	tic
	n = 16; %Any more seems to result in loss of color.
	img = round(img / n) * n;
	%figure(1);
	%imshow(img);
	%return;
	img24 = clr.toInt24(img);
	allNodes = clr.smash(img24);
	allEdges = clr.cncEdges(img24);
	nodeProps = clr.sortCount(allNodes);
	%edgeProps = sortCount(allEdges);
	
	
% 	length(nodeProps)
% 	length(edgeProps)
% 	toc
% 	return;
	%toc
	%return;
	for i = 1:length(nodeProps)
		allNodes(nodeProps(i).pixelList) = randi(256^3) - 1;
	end

	
	img24 = reshape(allNodes, [size(img24, 1), size(img24, 2)]);
	imgReproduced = clr.toRGB(img24);
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

%END
end
end

