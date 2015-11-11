colorArray = ...
[0, 0, 0; ...
0, 64, 128;  ...
0, 0, 255;   ...
0, 128, 255; ...
0, 255, 255; ...
0, 255, 0;   ...
255, 0, 0;   ...
128, 0, 128; ...
128, 128, 128; ...
200, 200, 200];
colorArray = [colorArray(:, 3) colorArray(:, 2) colorArray(:, 1)] / 255;

%all = [shapes; testshapes; train1; train2; match1; match2];
%all = [shapes; testshapes];
all = [train1; train2; match1; match2];
f1 = 'eccentricity';
f2 = 'rectDensity'; %***
f3 = 'scaledPerimeter';
f4 = 'hullDensity'; %*
f5 = 'density';
xlabel([f2 ' + ' f5]);
ylabel(f3);
%zlabel(f3);
hold on;
sums   = zeros([10 2]);
counts = zeros([10 1]);

z = 1

for i = 1:length(all);
	shapeNumber = all(i, 6);
	plot(all(i, 2) + all(i, 5), all(i, 3), '*', 'Color', colorArray(shapeNumber + 1, :));
	sums(shapeNumber + 1, :) = sums(shapeNumber + 1, :) + [all(i, 2) + all(i, 5), all(i, 3)];
	counts(shapeNumber + 1) = counts(shapeNumber + 1) + 1;
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

if sum(u1) < 1000, plot(u1(1), u1(2), 'o', 'Color', colorArray(1, :)); end
if sum(u2) < 1000, plot(u2(1), u2(2), 'o', 'Color', colorArray(2, :)); end
if sum(u3) < 1000, plot(u3(1), u3(2), 'o', 'Color', colorArray(3, :)); end
if sum(u4) < 1000, plot(u4(1), u4(2), 'o', 'Color', colorArray(4, :)); end
if sum(u5) < 1000, plot(u5(1), u5(2), 'o', 'Color', colorArray(5, :)); end
if sum(u6) < 1000, plot(u6(1), u6(2), 'o', 'Color', colorArray(6, :)); end
if sum(u7) < 1000, plot(u7(1), u7(2), 'o', 'Color', colorArray(7, :)); end
if sum(u8) < 1000, plot(u8(1), u8(2), 'o', 'Color', colorArray(8, :)); end
if sum(u9) < 1000, plot(u9(1), u9(2), 'o', 'Color', colorArray(9, :)); end
if sum(u10) < 1000, plot(u10(1), u10(2), 'o', 'Color', colorArray(10, :)); end


xc1 = [];
xc2 = [];
xc3 = [];
xc4 = [];
xc5 = [];
xc6 = [];
xc7 = [];
xc8 = [];
xc9 = [];
xc10 = [];

W = [15 1];

% for each point, calculate P(x | c_i)
ax = axis;
tickX = (ax(2) - ax(1)) / 100;
tickY = (ax(4) - ax(3)) / 100;
for x_1 = ax(1):tickX:ax(2)
	for x_2 = 3:tickY:12
		x = [x_1 x_2];
		d1 = norm((x - u1) .* W);
		d2 = norm((x - u2) .* W);
		d3 = norm((x - u3) .* W);
		d4 = norm((x - u4) .* W);
		d5 = norm((x - u5) .* W);
		d6 = norm((x - u6) .* W);
		d7 = norm((x - u7) .* W);
		d8 = norm((x - u8) .* W);
		d9 = norm((x - u9) .* W);
		d10 = norm((x - u10) .* W);
		
		dMin = min([d1 d2 d3 d4 d5 d6 d7 d8 d9 d10]);
		if     dMin == d1, xc1 = [xc1; x]; %This is about 10 times faster!
		elseif dMin == d2, xc2 = [xc2; x];
		elseif dMin == d3, xc3 = [xc3; x];
		elseif dMin == d4, xc4 = [xc4; x];
		elseif dMin == d5, xc5 = [xc5; x];
		elseif dMin == d6, xc6 = [xc6; x];
		elseif dMin == d7, xc7 = [xc7; x];
		elseif dMin == d8, xc8 = [xc8; x];
		elseif dMin == d9, xc9 = [xc9; x];
		elseif dMin == d10, xc10 = [xc10; x];
		end
	end
end

if ~isempty(xc1), plot(xc1(:, 1), xc1(:, 2), '.', 'Color', colorArray(1, :)); end
if ~isempty(xc2), plot(xc2(:, 1), xc2(:, 2), '.', 'Color', colorArray(2, :)); end
if ~isempty(xc3), plot(xc3(:, 1), xc3(:, 2), '.', 'Color', colorArray(3, :)); end
if ~isempty(xc4), plot(xc4(:, 1), xc4(:, 2), '.', 'Color', colorArray(4, :)); end
if ~isempty(xc5), plot(xc5(:, 1), xc5(:, 2), '.', 'Color', colorArray(5, :)); end
if ~isempty(xc6), plot(xc6(:, 1), xc6(:, 2), '.', 'Color', colorArray(6, :)); end
if ~isempty(xc7), plot(xc7(:, 1), xc7(:, 2), '.', 'Color', colorArray(7, :)); end
if ~isempty(xc8), plot(xc8(:, 1), xc8(:, 2), '.', 'Color', colorArray(8, :)); end
if ~isempty(xc9), plot(xc9(:, 1), xc9(:, 2), '.', 'Color', colorArray(9, :)); end
if ~isempty(xc10), plot(xc10(:, 1), xc10(:, 2), '.', 'Color', colorArray(10, :)); end