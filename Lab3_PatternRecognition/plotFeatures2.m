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

xlabel(f2);
ylabel(f4);
zlabel(f5);

hold on;
sums   = zeros([10 3]);
counts = zeros([10 1]);

usedFeatures = zeros(length(all), 3);
for shapeNumber = 0:9
	xc = [];
	for i = 1:length(all);
		if shapeNumber == all(i, 6)
			if choice == 0
				usedFeatures(i, :) = [all(i, 2), all(i, 3), all(i, 5)];
			else
				usedFeatures(i, :) = [all(i, 2), all(i, 4), all(i, 5)];
			end
			xc = [xc; usedFeatures(i, :)];
			sums(shapeNumber + 1, :) = sums(shapeNumber + 1, :) + usedFeatures(i, :);
			counts(shapeNumber + 1) = counts(shapeNumber + 1) + 1;
		end
	end
	if ~isempty(xc)
		plot3(xc(:, 1), xc(:, 2), xc(:, 3), '*', 'Color', colorArray(shapeNumber + 1, :));
	end
end

W = 1./(max(usedFeatures) - min(usedFeatures));
clusters = [usedFeatures ones([length(usedFeatures) 1])];

while true
	%find minimum distance between any two points, any two clusters, or any
	%cluster and point
	dist = zeros([nchoosek(length(clusters), 2) 3]);
	k = 0;
	for i = 1:length(clusters)
		for j = (i+1):length(clusters)
			k = k + 1;
			d = norm((clusters(i, 1:3) - clusters(j, 1:3)) .* W);
			dist(k, :) = [i j d];
		end
	end
	[~, k] = min(dist(:, 3));
	k;
	i = dist(k, 1);
	j = dist(k, 2);
	ithCluster = clusters(i, :);
    jthCluster = clusters(j, :);
	totalMass = ithCluster(4) + jthCluster(4);
	centerOfMass = (ithCluster(1:3).*ithCluster(4) + jthCluster(1:3).*jthCluster(4)) / totalMass;
	newCluster = [centerOfMass totalMass];
	clusters(j, :) = [newCluster];
	clusters(i, :) = [];
	xc = [ithCluster; jthCluster];
	plot3(xc(:, 1), xc(:, 2), xc(:, 3), 'k');	

	%repeat until only 5 clusters remain.
	if length(clusters) <= 6
		break;
	end
end
for i = 1:length(clusters)
	plot3(clusters(:, 1), clusters(:, 2), clusters(:, 3), 'ok');
end
