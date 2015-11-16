if isempty(shapes)
	featureData
end

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
	all = [shapes]; %; testshapes];
else
	all = [train1];%; train2; match1; match2];
end
f1 = 'eccentricity';
f2 = 'rectangular bounding box density';
f3 = 'normalized perimeter';
f4 = 'convex hull density';
f5 = 'density';
f6 = 'shift';

if choice == 0
	xlabel(f3);
	ylabel(f5);
else
	xlabel(f2);
	ylabel(f4);
	zlabel(f5);
end




hold on;

n = 3;
usedFeatures = zeros(length(all), n);
sums   = zeros([10 n]);
counts = zeros([10 1]);
for shapeNumber = 0:9
	xc = [];
	for i = 1:length(all);
		if shapeNumber == all(i, 7)
			if choice == 0
				usedFeatures(i, :) = [all(i, 3), all(i, 5)];
			else
				usedFeatures(i, :) = [all(i, 2), all(i, 4), all(i, 5)];
			end
			xc = [xc; usedFeatures(i, :)];
			sums(shapeNumber + 1, :) = sums(shapeNumber + 1, :) + usedFeatures(i, :);
			counts(shapeNumber + 1) = counts(shapeNumber + 1) + 1;
		end
	end
	if ~isempty(xc)		
		if n == 1
			plot(xc(:, 1), shapeNumber, '*', 'Color', colorArray(shapeNumber + 1, :));
		elseif n == 2
			plot(xc(:, 1), xc(:, 2), '*', 'Color', colorArray(shapeNumber + 1, :));
		else
			plot3(xc(:, 1), xc(:, 2), xc(:, 3), '*', 'Color', colorArray(shapeNumber + 1, :));
		end
	end
end

W = 1./(max(usedFeatures) - min(usedFeatures));
clusters = [usedFeatures ones([size(usedFeatures, 1) 1])];
size(clusters);

while true
	%find minimum distance between any two points, any two clusters, or any
	%cluster and point
	dist = zeros([nchoosek(size(clusters, 1), 2) 3]);
	k = 0;
	for i = 1:size(clusters, 1)
		for j = (i+1):size(clusters, 1)
			k = k + 1;
			d = norm((clusters(i, 1:n) - clusters(j, 1:n)) .* W);
			dist(k, :) = [i j d];
		end
	end
	[~, k] = min(dist(:, 3));
	k;
	i = dist(k, 1);
	j = dist(k, 2);
	ithCluster = clusters(i, :);
    jthCluster = clusters(j, :);
	totalMass = ithCluster(n + 1) + jthCluster(n + 1);
	centerOfMass = (ithCluster(1:n).*ithCluster(n + 1) + jthCluster(1:n).*jthCluster(n + 1)) / totalMass;
	newCluster = [centerOfMass totalMass];
	clusters(j, :) = newCluster;
	clusters(i, :) = [];
	xc = [ithCluster; jthCluster];
	if n == 2
		plot(xc(:, 1), xc(:, 2), 'k');
	else
		plot3(xc(:, 1), xc(:, 2), xc(:, 3), 'k');
	end

	%repeat until only 5 clusters remain.
	if size(clusters, 1) <= 5
		break;
	end
end
for i = 1:size(clusters, 1)
	plot3(clusters(:, 1), clusters(:, 2), clusters(:, 3), 'ok');
end

if choice == 0
	legend('rectangle', 'triangle', 'circle', 'millstone', 'oval')
else
	legend('millstone', 'rounded cross', 'oval', 'E', 'F')
end
