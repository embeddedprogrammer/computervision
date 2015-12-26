%mapObj = containers.Map('KeyType', 'int64', 'ValueType', 'int32');
tic
vals = randi(500, [50000 1]);
props = sortCount(vals);

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