function o = get_option(options, fname, def)
	if isfield(options, fname)
		o = options.(fname);
	else
		o = def;
	end
end