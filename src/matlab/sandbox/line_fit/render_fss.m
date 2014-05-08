function [] = render_fss(scan, line_models)
	% render_fss(scan)
	% render_fss(scan, line_models)
	%
	%	Will render the provided scan along with a set
	%	of line models found by the function line_fit(scan)
	%

	% check for optional args
	if(~exist('line_models', 'var'))
		line_models = [];
	end

	% prepare figure
	clf;
	hold all;
	axis equal;
	title(['Timestamp: ', num2str(scan.timestamp)], 'FontSize', 18);

	% for the sake of simplicity, rotate the view so the downscanner
	% is oriented correctly
	xs = scan.pts(2,:);
	ys = -scan.pts(1,:);

	% make each line model a different color
	cs = zeros(size(xs)); % colors initialized to zero
	for i = 1:length(line_models)
		% i'th line gets unique color
		cs(line_models(i).inliers) = i;
	end

	% render the scan points
	scatter(xs, ys, 10, cs);
	colorbar;
end
