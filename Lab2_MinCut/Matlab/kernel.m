% function trc()
% 	d = 1:512;
% 	c = cost(d);
% 	plot(d, c);
% end
function kernel(name)
	img = imread(name);
	imshow(img);
% 	subplot(221); imshow(img);
% 	subplot(222); applyKernel(img, [1 -1]);
% 	subplot(223); applyKernel(img, [1 -1; 1 -1; 1 -1; 1 -1] / 4);
% 	subplot(224); medKernel(img, [3 1]);
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
function medKernel(img, rc)
	imgMed(:,:,1) = medfilt2(img(:,:,1), rc);
	imgMed(:,:,2) = medfilt2(img(:,:,2), rc);
	imgMed(:,:,3) = medfilt2(img(:,:,3), rc);
	applyKernel(imgMed, [1 -1]);
end
function applyKernel(img, k)
	imgFloat = single(img);
	imgConv(:,:,1) = conv2(imgFloat(:,:,1), k);
	imgConv(:,:,2) = conv2(imgFloat(:,:,2), k);
	imgConv(:,:,3) = conv2(imgFloat(:,:,3), k);
	imshow(cost(rgb2gray(uint8(abs(imgConv)))));
end
function c = cost(d)
	c = uint8(255 ./ (d + 1));
% 	u = 3;
% 	B = 3;
% 	x = double(d);
% 	c = uint8((atan(-(x - u)/(B/4))/pi+.5)*256);
end
