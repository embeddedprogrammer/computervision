colorArray = ...
[0, 0, 0; ...
0, 74, 148;  ...
0, 0, 255;   ...
0, 128, 255; ...
0, 255, 255; ...
0, 255, 0;   ...
255, 0, 0;   ...
128, 0, 128; ...
128, 128, 128; ...
200, 200, 200];
colorArray = [colorArray(:, 3) colorArray(:, 2) colorArray(:, 1)] / 255;

choice = 1;
if choice == 0
	%all = [shapes; testshapes; train1; train2; match1; match2];
	all = [shapes; testshapes];
else
	all = [train1; train2; match1; match2];
end
f1 = 'eccentricity';
f2 = 'rectangular bounding box density'; %***
f3 = 'normalized perimeter';
f4 = 'convex hull density'; %*
f5 = 'density';

if choice == 0
	xlabel(f3);
	ylabel(f5);
else
	xlabel([f2 ' + ' f5]);
	ylabel(f3);
end

hold on;
sums   = zeros([10 2]);
counts = zeros([10 1]);

usedFeatures = zeros(length(all), 2);
for shapeNumber = 0:9
	xc = [];
	for i = 1:length(all);
		if shapeNumber == all(i, 6)
			if choice == 0
				usedFeatures(i, :) = [all(i, 3), all(i, 5)];
			else
				usedFeatures(i, :) = [all(i, 2) + all(i, 5), all(i, 3)];
			end
			xc = [xc; usedFeatures(i, :)];
			sums(shapeNumber + 1, :) = sums(shapeNumber + 1, :) + usedFeatures(i, :);
			counts(shapeNumber + 1) = counts(shapeNumber + 1) + 1;
		end
	end
	if ~isempty(xc)
		plot(xc(:, 1), xc(:, 2), '*', 'Color', colorArray(shapeNumber + 1, :));
	end
end

if counts(1) >= 1, u1 = sums(1, :) / counts(1); else u1 = [1 1] * 1000; end
if counts(2) >= 1, u2 = sums(2, :) / counts(2); else u2 = [1 1] * 1000; end
if counts(3) >= 1, u3 = sums(3, :) / counts(3); else u3 = [1 1] * 1000; end
if counts(4) >= 1, u4 = sums(4, :) / counts(4); else u4 = [1 1] * 1000; end
if counts(5) >= 1, u5 = sums(5, :) / counts(5); else u5 = [1 1] * 1000; end
if counts(6) >= 1, u6 = sums(6, :) / counts(6); else u6 = [1 1] * 1000; end
if counts(7) >= 1, u7 = sums(7, :) / counts(7); else u7 = [1 1] * 1000; end
if counts(8) >= 1, u8 = sums(8, :) / counts(8); else u8 = [1 1] * 1000; end
if counts(9) >= 1, u9 = sums(9, :) / counts(9); else u9 = [1 1] * 1000; end
if counts(10) >= 1, u10 = sums(10, :) / counts(10); else u10 = [1 1] * 1000; end
u = [u1; u2; u3; u4; u5; u6; u7; u8; u9; u10];

xc = [];

W = 1./(max(usedFeatures) - min(usedFeatures));

if choice == 0
	ylim([.1 1.1])
end
ax = axis;
divisions = 500;
tickX = (ax(2) - ax(1)) / divisions;
tickY = (ax(4) - ax(3)) / divisions;
d = zeros([1 10]);
s = zeros([divisions divisions]);
for x_2 = 1:divisions
	for x_1 = 1:divisions
		x = [ax(1)+tickX*x_1 ax(3)+tickY*x_2];
		for i = 1:10
			d(i) = norm((x - u(i, :)) .* W);
		end
		[~, region] = min(d);
		s(x_1, x_2) = region;
		if (x_1 > 1 && region ~= s(x_1 - 1, x_2)) || ...
			(x_2 > 1 && region ~= s(x_1, x_2 - 1))
			xc = [xc; x];
		end
	end
end

plot(xc(:, 1), xc(:, 2), '.k');

if choice == 0
	legend('rectangle', 'triangle', 'circle', 'millstone', 'oval');
else
	legend('millstone', 'rounded cross', 'oval', 'E', 'F');
end