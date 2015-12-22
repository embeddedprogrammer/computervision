function Untitled
	vid = VideoReader('C:\Users\Jacob\Documents\Pen drive\Ecen 490\Pics and videos\gameNum0.avi');
	img = readFrame(vid);
	imwrite(img, 'soccer.png')
	imshow(img);
	return;
	
	clear vid;

	toInt24 = @(r, g, b) r*256*256 + g*256 + b;
	toRGB = @(v) [mod(floor(v/256/256), 256) mod(floor(v / 256), 256) mod(v, 256)];

	%plot([1 7], [2 5], 'b');
	imshow(img);

	ax = get(gcf, 'children');

	%sbp = subplot('Position', [.25 .1 0.7 0.85]);

	%set(gcf, 'ButtonDownFcn', 'pos = get(ax,''CurrentPoint''), calcpoint = 1;');
	%subplot(111);
	%set(gcf, 'WindowButtonDownFcn', 'pos = get(ax,''CurrentPoint''); pos([1 3])');
	set(gcf, 'WindowButtonDownFcn', @mouseCb);
	
	function key = mouseCb(~, callbackdata)
		pos = get(ax(1),'CurrentPoint');
		pos([1 3])
	end




	%= containers.Map('KeyType','int32','ValueType','int32')

	return;


	% create pixel list
	n = 16;
	img2 = round(img / n) * n;
	allClrs = reshape(img2, [size(img, 1) * size(img, 2), 3]);
	clrs = unique(allClrs, 'rows');
	%size(clrs)
	clf;
	hold on;

	n = 8;
	[x,y,z] = sphere(n);
	hsvclr = shiftdim(rgb2hsv(shiftdim(clrs, -1)), 1);
	for i = 2:size(clrs, 1)
		%plot3(hsvclr(i, 1), hsvclr(i, 2), hsvclr(i, 3), '*', 'color', [clrs(i, 1) clrs(i, 2) clrs(i, 3)] * 16);
		count = sum((allClrs(:, 1) == clrs(i, 1)) & (allClrs(:, 2) == clrs(i, 2)) & (allClrs(:, 3) == clrs(i, 3)));
		sphereSize = count^(1/5) * .01;

		C = [];
		C(:,:,1) = ones(n + 1) * double(clrs(i, 1)) / 256;
		C(:,:,2) = ones(n + 1) * double(clrs(i, 2)) / 256;
		C(:,:,3) = ones(n + 1) * double(clrs(i, 3)) / 256;
		surf(hsvclr(i, 1) + x*sphereSize, hsvclr(i, 2) + y*sphereSize, hsvclr(i, 3) + z*sphereSize, C, ...
			'EdgeColor', clrs(i, :));
	end
	xlabel('Hue');
	ylabel('Sat');
	zlabel('Val');

	% cm = sparse([1 1 2 2 3 3 4 5],[2 3 4 5 4 5 6 6],...
	%      [2 3 3 1 1 1 2 3],6,6)
	% [maxFlow,flowMatrix,K] = graphmaxflow(cm,1,6)
	% h = view(biograph(cm,[],'ShowWeights','on'))
	% set(h.Nodes(K(1,:)),'Color',[1 0 0])
end
function buildGraph(img)
	for c=1:size(img, 2)-1
		for r=1:size(img, 1)-1
			c11 = img(r, c);
			c12 = img(r, c + 1);
			c21 = img(r + 1, c);
		end
	end
end





