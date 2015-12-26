function newCostMap(name)
	tic
	img = imread(name);
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
	costMap = calcEdgeNodeProps(nodeProps, edgeProps);
	
	costImg = zeros([size(img, 1) size(img, 2)], 'uint8');
	for c=1:size(img, 2)
		for r=1:size(img, 1)
			if r < size(img, 1)
				%cst = cost(img24(r, c), img24(r+1, c), nodeProps, edgeProps);
				cst = costMap(clrToInt64(img24(r, c), img24(r+1, c)));
				%cst = costOld(img(r, c, :), img(r+1, c, :));
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r+1, c) = costImg(r+1, c) + cst;
			end
			if c < size(img, 2)
				%cst = cost(img24(r, c), img24(r, c+1), nodeProps, edgeProps);
				cst = costMap(clrToInt64(img24(r, c), img24(r, c+1)));
				%cst = costOld(img(r, c, :), img(r, c+1, :));
				costImg(r, c) = costImg(r, c) + cst;
				costImg(r, c+1) = costImg(r, c+1) + cst;
			end
		end
	end
	imshow(costImg);
	toc;
end
function cst = cost(v1, v2, nodeProps, edgeProps)
	a = nodeProps([nodeProps.clr] == v1).count;
	b = nodeProps([nodeProps.clr] == v2).count;
	c = edgeProps([edgeProps.clr] == clrToInt64(v1, v2)).count;
	cst = uint8(100*c^2/(a*b));
end
function v = clrToInt64(a, b)
	v = int64(a)*256*256*256 + int64(b);
end
function [a, b] = fromInt64(v)
	b = mod(v, 256*256*256);
	v = (v - b) / (256*256*256);
	a = v;
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
function costMap = calcEdgeNodeProps(nodeProps, edgeProps)
	nodeMap = containers.Map([nodeProps.clr], [nodeProps.count]);
	cst = zeros(size(edgeProps), 'uint8');
	for i=1:length(edgeProps)
		edgeProp = edgeProps(i);
		[a, b] = fromInt64(edgeProp.clr);
		aCnt = nodeMap(a);
		bCnt = nodeMap(b);
		cCnt = edgeProp.count;
		cst(i) = 20*cCnt^2/(aCnt*bCnt);
	end
	costMap = containers.Map([edgeProps.clr], cst);
end
function d = clrDiff(v1, v2)
	d = sum(abs(int16(v1 - v2)));
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
function cncClrs = cncEdges(img)
	 cncClrs = [clrToInt64(smash(img(:, 1:size(img,2)-1)), smash(img(:, 2:size(img,2))))
				clrToInt64(smash(img(1:size(img,1)-1, :)), smash(img(2:size(img,1), :)))
				clrToInt64(smash(img(:, 2:size(img,2))), smash(img(:, 1:size(img,2)-1)))
				clrToInt64(smash(img(2:size(img,1), :)), smash(img(1:size(img,1)-1, :)))];
end