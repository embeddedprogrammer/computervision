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