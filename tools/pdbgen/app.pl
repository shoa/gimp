# The GIMP -- an image manipulation program
# Copyright (C) 1998-2003 Manish Singh <yosh@gimp.org>

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

package Gimp::CodeGen::app;

$destdir = "$main::destdir/app/pdb";

*arg_types = \%Gimp::CodeGen::pdb::arg_types;
*arg_parse = \&Gimp::CodeGen::pdb::arg_parse;
*arg_ptype = \&Gimp::CodeGen::pdb::arg_ptype;
*arg_vname = \&Gimp::CodeGen::pdb::arg_vname;

*enums = \%Gimp::CodeGen::enums::enums;

*write_file = \&Gimp::CodeGen::util::write_file;
*FILE_EXT   = \$Gimp::CodeGen::util::FILE_EXT;

use Text::Wrap qw(wrap);

sub quotewrap {
    my ($str, $indent) = @_;
    my $leading = ' ' x $indent . '"';
    $Text::Wrap::columns = 1000;
    $str = wrap($leading, $leading, $str);
    $str =~ s/^\s*//s;
    $str =~ s/(.)$/$1"/mg;
    $str;
}

sub format_code_frag {
    my ($code, $indent) = @_;

    chomp $code;
    $code =~ s/\t/' ' x 8/eg;

    if (!$indent && $code =~ /^\s*{\s*\n.*\n\s*}\s*$/s) {
	$code =~ s/^\s*{\s*\n//s;
	$code =~ s/\n\s*}\s*$//s;
    }
    else {
	$code =~ s/^/' ' x ($indent ? 4 : 2)/meg;
    }
    $code .= "\n";

    $code =~ s/^\s+$//mg;

    $code;
}

sub arg_value {
    my ($arg, $argc) = @_;
    my $cast = "";

    my $type = &arg_ptype($arg);

    if ($type eq 'pointer' || $arg->{type} =~ /int(16|8)$/) {
	$cast = "($arg->{type}) ";
    }

    return "${cast}args[$argc].value.pdb_$type";
}

sub make_arg_test {
    my ($arg, $reverse, $test) = @_;
    my $result = "";

    if (!exists $arg->{no_success}) {
	if (exists $arg->{cond}) {
	    my $cond = "";
	    foreach (@{$arg->{cond}}) {
		$cond .= /\W/ ? "($_)" : $_;
		$cond .= ' && ';
	    }
	    $test = "$cond($test)";
	}

	$result = ' ' x 2 . "if ($test)\n";

	$success = 1;
	$result .= ' ' x 4 . "success = FALSE;\n";
    }

    $result;
}

sub declare_args {
    my $proc = shift;
    my $out = shift;
    my $init = shift;

    local $result = "";

    foreach (@_) {
	my @args = @{$proc->{$_}} if exists $proc->{$_};

	foreach (@args) {
	    my ($type, $name) = &arg_parse($_->{type});
	    my $arg = $arg_types{$type};

	    if ($arg->{array} && !exists $_->{array}) {
		warn "Array without number of elements param in $proc->{name}";
	    }

	    unless (exists $_->{no_declare}) {
		$result .= ' ' x 2 . $arg->{type} . &arg_vname($_);
		if ($init) {
		    $result .= " = $arg->{init_value}";
		}
		$result .= ";\n";

		if (exists $arg->{headers}) {
		    foreach (@{$arg->{headers}}) {
			$out->{headers}->{$_}++;
		    }
		}
	    }
	}
    }

    $result;
}

sub make_arg_recs {
    my $proc = shift;

    my $result = "";
    my $once;

    foreach (@_) {
	my @args = @{$proc->{$_}} if exists $proc->{$_};

	if (scalar @args) {
	    $result .= "\nstatic ProcArg $proc->{name}_${_}\[] =\n{\n";

	    foreach $arg (@{$proc->{$_}}) {
		my ($type, $name, @remove) = &arg_parse($arg->{type});
		my $desc = $arg->{desc};
		my $info = $arg->{type};

		for ($type) {
		    /array/     && do { 				 last };
		    /boolean/   && do { $info = 'TRUE or FALSE';	 last };
		    /int|float/ && do { $info =~ s/$type/$arg->{name}/e; last };
		    /enum/      && do { my $enum = $enums{$name};
					$info = $enum->{info};
					foreach (@remove) {
					    $info =~ s/$_ \(.*?\)(, )?//
					}				 
					$info =~ s/, $//;
					if (!$#{[$info =~ /,/g]} &&
					     $desc !~ /{ %%desc%% }/) {
					    $info =~ s/,/ or/
					}				 last };
		}

		$desc =~ s/%%desc%%/$info/eg;

		$result .= <<CODE;
  {
    GIMP_PDB_$arg_types{$type}->{name},
    "$arg->{canonical_name}",
    @{[ &quotewrap($desc, 4) ]}
  },
CODE
	    }

	    $result =~ s/,\n$/\n/s;
	    $result .= "};\n";
	}
    }

    $result;
}

sub marshal_inargs {
    my ($proc, $argc) = @_;

    my $result = "";
    my %decls;

    my @inargs = @{$proc->{inargs}} if exists $proc->{inargs};

    foreach (@inargs) {
	my($pdbtype, @typeinfo) = &arg_parse($_->{type});
	my $arg = $arg_types{$pdbtype};
	my $var = &arg_vname($_);
	my $value = &arg_value($arg, $argc++);
	
	if (exists $arg->{id_func}) {
	    my $id_func = $arg->{id_func};
	    $id_func = $_->{id_func} if exists $_->{id_func};

	    $result .= "  $var = $id_func (gimp, $value);\n";

	    if (exists $arg->{check_func}) {
		my $check_func = eval qq/"$arg->{check_func}"/;

		$result .= &make_arg_test($_, sub { ${$_[0]} =~ s/==/!=/ },
		                          "! $check_func");
	    } else {
		$result .= &make_arg_test($_, sub { ${$_[0]} =~ s/==/!=/ },
				          "$var == NULL");
	    }
	}
	else {
	    $result .= ' ' x 2 . "$var = $value";
	    $result .= ' ? TRUE : FALSE' if $pdbtype eq 'boolean';
	    $result .= ";\n";

	    if ($pdbtype eq 'string' || $pdbtype eq 'parasite') {
		my ($reverse, $test, $utf8, $utf8testvar);

		$test = "$var == NULL";
                $utf8 = 1;

		if ($pdbtype eq 'parasite') {
		    $test .= " || $var->name == NULL";
		    $utf8testvar = "$var->name";
		}
		else {
		    $utf8 = !exists $_->{no_validate};
		    $utf8testvar = "$var";
		}

		if (exists $_->{null_ok}) {
		    $reverse = sub { ${$_[0]} =~ s/!//; };
		    $test = "$var && !g_utf8_validate ($var, -1, NULL)";
		}
		elsif ($utf8) {
		    $reverse = sub { ${$_[0]} =~ s/!//;
				     ${$_[0]} =~ s/||/&&/g;
				     ${$_[0]} =~ s/==/!=/g };
                    $test .= " || !g_utf8_validate ($utf8testvar, -1, NULL)";
		}
		else {
		    $reverse = sub { ${$_[0]} =~ s/||/&&/g;
				     ${$_[0]} =~ s/==/!=/g };
		}

		$result .= &make_arg_test($_, $reverse, $test);
	    }
	    elsif ($pdbtype eq 'tattoo') {
		$result .= &make_arg_test($_, sub { ${$_[0]} =~ s/==/!=/ },
					  "$var == 0");
	    }
	    elsif ($pdbtype eq 'unit') {
		$typeinfo[0] = 'GIMP_UNIT_PIXEL' unless defined $typeinfo[0];
		$result .= &make_arg_test($_, sub { ${$_[0]} = "!(${$_[0]})" },
					  "$var < $typeinfo[0] || $var >= " .
					  '_gimp_unit_get_number_of_units (gimp)');
	    }
	    elsif ($pdbtype eq 'enum' && !$enums{$typeinfo[0]}->{contig}) {
		if (!exists $_->{no_success}) {
		    my %vals; my $symbols = $enums{pop @typeinfo}->{symbols};
		    @vals{@$symbols}++; delete @vals{@typeinfo};

		    my $okvals = ""; my $failvals = "";

		    my $once = 0;
		    foreach (@$symbols) {
			if (exists $vals{$_}) {
			    $okvals .= ' ' x 4 if $once++;
			    $okvals .= "case $_:\n";
			}
		    }

		    $failvals .= "default:\n";
		    if (!exists $_->{no_success}) {
			$success = 1;
			$failvals .= ' ' x 6 . "success = FALSE;\n"
		    }

		    $result .= <<CODE;
  switch ($var)
    {
    $okvals
      break;

    $failvals
      break;
    }
CODE
		}
	    }
	    elsif (defined $typeinfo[0] || defined $typeinfo[2]) {
		my $code = ""; my $tests = 0; my $extra = "";

		if ($pdbtype eq 'enum') {
		    my $symbols = $enums{shift @typeinfo}->{symbols};

		    my ($start, $end) = (0, $#$symbols);

		    my $syms = "@$symbols "; my $test = $syms;
		    foreach (@typeinfo) { $test =~ s/$_ // }

		    if ($syms =~ /$test/g) {
			if (pos $syms  == length $syms) {
			    $start = @typeinfo;
			}
			else {
			    $end -= @typeinfo;
			}
		    }
		    else {
			foreach (@typeinfo) {
			    $extra .= " || $var == $_";
			}
		    }

		    $typeinfo[0] = $symbols->[$start];
		    if ($start != $end) {
			$typeinfo[1] = '<';
			$typeinfo[2] = $symbols->[$end];
			$typeinfo[3] = '>';
		    }
		    else {
			$typeinfo[1] = '!=';
			undef @typeinf[2..3];
		    }
		}
		elsif ($pdbtype eq 'float') {
		    foreach (@typeinfo[0, 2]) {
			$_ .= '.0' if defined $_ && !/\./
		    }
		}

		if (defined $typeinfo[0]) {
		    $code .= "$var $typeinfo[1] $typeinfo[0]";
		    $code .= '.0' if $pdbtype eq 'float' && $typeinfo[0] !~ /\./;
		    $tests++;
		}

		if (defined $typeinfo[2]) {
		    $code .= ' || ' if $tests;
		    $code .= "$var $typeinfo[3] $typeinfo[2]";
		}

		$code .= $extra;

		$result .= &make_arg_test($_, sub { ${$_[0]} = "!(${$_[0]})" },
					  $code);
	    }
	}

	$result .= "\n";
    }

    $result = "\n" . $result if $result;
    $result;
}

sub marshal_outargs {
    my $proc = shift;

    my $result = <<CODE;
  return_args = procedural_db_return_args (\&$proc->{name}_proc, success);
CODE

    my $argc = 0;
    my @outargs = @{$proc->{outargs}} if exists $proc->{outargs};

    if (scalar @outargs) {
	my $outargs = "";

	foreach (@{$proc->{outargs}}) {
	    my ($pdbtype) = &arg_parse($_->{type});
	    my $arg = $arg_types{$pdbtype};
	    my $type = &arg_ptype($arg);
	    my $var = &arg_vname($_);

	    $argc++; $outargs .= ' ' x 2;

            if (exists $arg->{id_ret_func}) {
		$var = eval qq/"$arg->{id_ret_func}"/;
	    }

	    $outargs .= "return_args[$argc].value.pdb_$type = $var;\n";
	}

	$outargs =~ s/^/' ' x 2/meg if $success;
	$outargs =~ s/^/' ' x 2/meg if $success && $argc > 1;

	$result .= "\n" if $success || $argc > 1;
	$result .= ' ' x 2 . "if (success)\n" if $success;
	$result .= ' ' x 4 . "{\n" if $success && $argc > 1;
	$result .= $outargs;
	$result .= ' ' x 4 . "}\n" if $success && $argc > 1;
        $result .= "\n" . ' ' x 2 . "return return_args;\n";
    }
    else {
	$result =~ s/_args =//;
    }

    $result =~ s/, success\);$/, TRUE);/m unless $success;
    $result;
}

sub generate {
    my @procs = @{(shift)};
    my %out;
    my $total = 0.0;

    foreach $name (@procs) {
	my $proc = $main::pdb{$name};
	my $out = \%{$out{$proc->{group}}};

	my @inargs = @{$proc->{inargs}} if exists $proc->{inargs};
	my @outargs = @{$proc->{outargs}} if exists $proc->{outargs};
	
	local $success = 0;

	$out->{pcount}++; $total++;

	$out->{procs} .= "static ProcRecord ${name}_proc;\n";

	$out->{register} .= <<CODE;
  procedural_db_register (gimp, \&${name}_proc);
CODE

	if (exists $proc->{invoke}->{headers}) {
	    foreach $header (@{$proc->{invoke}->{headers}}) {
		$out->{headers}->{$header}++;
	    }
	}

	$out->{code} .= "\nstatic Argument *\n";
	$out->{code} .= "${name}_invoker (Gimp         *gimp,\n";
	$out->{code} .=  ' ' x length($name) . "          GimpContext  *context,\n";
	$out->{code} .=  ' ' x length($name) . "          GimpProgress *progress,\n";
	$out->{code} .=  ' ' x length($name) . "          Argument     *args)\n{\n";

	my $code = "";

	if (exists $proc->{invoke}->{proc}) {
	    my ($procname, $args) = @{$proc->{invoke}->{proc}};
	    my ($exec, $fail, $argtype);
	    my $custom = $proc->{invoke}->{code};

	    $exec = "procedural_db_execute (gimp, context, progress, $procname, $args)";
	    $fail = "procedural_db_return_args (\&${name}_proc, FALSE)";

	    $argtype = 'Argument';
	    if (exists $proc->{invoke}->{args}) {
		foreach (@{$proc->{invoke}->{args}}) {
		    $code .= "  $argtype *$_;\n";
		}
	    }

	    foreach (qw(exec fail argtype)) { $custom =~ s/%%$_%%/"\$$_"/eeg }

	    my $pos = 0;
	    foreach (@{$proc->{inargs}}) {
		my $arg = $arg_types{(&arg_parse($_->{type}))[0]};
		my $var = &arg_vname($_);
		$custom =~ s/%%$var%%/&arg_value($arg, $pos)/e;
		$pos++;
	    }

	    $code .= "\n" if length($code);
	    $code .= &format_code_frag($custom, 0) . "}\n";
	}
	elsif (exists $proc->{invoke}->{pass_through}) {
	    my $invoke = $proc->{invoke};

	    my $argc = 0;
	    $argc += @{$invoke->{pass_args}} if exists $invoke->{pass_args};
	    $argc += @{$invoke->{make_args}} if exists $invoke->{make_args};

	    my %pass; my @passgroup;
	    my $before = 0; my $contig = 0; my $pos = -1;
	    if (exists $invoke->{pass_args}) {
		foreach (@{$invoke->{pass_args}}) {
		    $pass{$_}++;
		    $_ - 1 == $before ? $contig = 1 : $pos++;
		    push @{$passgroup[$pos]}, $_;
		    $before = $_;
		}
	    } 
	    $code .= ' ' x 2 . "int i;\n" if $contig;

	    $code .= ' ' x 2 . "Argument argv[$argc];\n";

	    my $tempproc; $pos = 0;
	    foreach (@{$proc->{inargs}}) {
		$_->{argpos} = $pos++;
		push @{$tempproc->{inargs}}, $_ if !exists $pass{$_->{argpos}};
	    }

	    $code .= &declare_args($tempproc, $out, 0, qw(inargs)) . "\n";

	    my $marshal = "";
	    foreach (@{$tempproc->{inargs}}) {
		my $argproc; $argproc->{inargs} = [ $_ ];
		$marshal .= &marshal_inargs($argproc, $_->{argpos});
		chop $marshal;
	    }
	    $marshal .= "\n" if $marshal;

	    if ($success) {
		$marshal .= <<CODE;
  if (!success)
    return procedural_db_return_args (\&${name}_proc, FALSE);

CODE
	    }

	    $marshal = substr($marshal, 1) if $marshal;
	    $code .= $marshal;

	    foreach (@passgroup) {
		$code .= ($#$_ ? <<LOOP : <<CODE) . "\n";
  for (i = $_->[0]; i < @{[ $_->[$#$_] + 1 ]}; i++)
    argv[i] = args[i];
LOOP
  argv[$_->[0]] = args[$_->[0]];
CODE
	    }

	    if (exists $invoke->{make_args}) {
		$pos = 0;
		foreach (@{$invoke->{make_args}}) {
		    while (exists $pass{$pos}) { $pos++ }
		    
		    my $arg = $arg_types{(&arg_parse($_->{type}))[0]};
		    my $type = &arg_ptype($arg);

		    $code .= <<CODE;
  argv[$pos].arg_type = GIMP_PDB_$arg->{name};
CODE

		    my $frag = $_->{code};
		    $frag =~ s/%%arg%%/"argv[$pos].value.pdb_$type"/e;
		    $code .= &format_code_frag($frag, 0);

		    $pos++;
		}
		$code .= "\n";
	    }

	    $code .= <<CODE;
  return $invoke->{pass_through}_invoker (gimp, context, progress, argv);
}
CODE
	}
	else {
	    my $invoker = "";
	
	    $invoker .= ' ' x 2 . "Argument *return_args;\n" if scalar @outargs;
	    $invoker .= &declare_args($proc, $out, 0, qw(inargs));
	    $invoker .= &declare_args($proc, $out, 1, qw(outargs));

	    $invoker .= &marshal_inargs($proc, 0);
	    $invoker .= "\n" if $invoker && $invoker !~ /\n\n/s;

	    my $frag = "";

	    if (exists $proc->{invoke}->{code}) {
		$frag = &format_code_frag($proc->{invoke}->{code}, $success);
		$frag = ' ' x 2 . "if (success)\n" . $frag if $success;
		$success = ($frag =~ /success =/) unless $success;
	    }

	    chomp $invoker if !$frag;
	    $code .= $invoker . $frag;
	    $code .= "\n" if $frag =~ /\n\n/s || $invoker;
	    $code .= &marshal_outargs($proc) . "}\n";
	}

	if ($success) {
	    $out->{code} .= ' ' x 2 . "gboolean success";
	    unless ($proc->{invoke}->{success} eq 'NONE') {
		$out->{code} .= " = $proc->{invoke}->{success}";
	    }
	    $out->{code} .= ";\n";
	}

        $out->{code} .= $code;

	$out->{code} .= &make_arg_recs($proc, qw(inargs outargs));

	$out->{code} .= <<CODE;

static ProcRecord ${name}_proc =
{
  "gimp-$proc->{canonical_name}",
  "gimp-$proc->{canonical_name}",
  @{[ &quotewrap($proc->{blurb}, 2) ]},
  @{[ &quotewrap($proc->{help},  2) ]},
  "$proc->{author}",
  "$proc->{copyright}",
  "$proc->{date}",
  @{[$proc->{deprecated} ? "\"$proc->{deprecated}\"" : 'NULL']},
  GIMP_INTERNAL,
  @{[scalar @inargs]},
  @{[scalar @inargs ? "${name}_inargs" : 'NULL']},
  @{[scalar @outargs]},
  @{[scalar @outargs ? "${name}_outargs" : 'NULL']},
  { { ${name}_invoker } }
};
CODE
    }

    my $gpl = <<'GPL';
/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-2003 Spencer Kimball and Peter Mattis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/* NOTE: This file is autogenerated by pdbgen.pl. */

GPL

    my $group_procs = "";
    my $longest = 0;
    my $once = 0;
    my $pcount = 0.0;

    foreach $group (@main::groups) {
	my $out = $out{$group};

	foreach (@{$main::grp{$group}->{headers}}) { $out->{headers}->{$_}++ }
	delete $out->{headers}->{q/"procedural_db.h"/};
	delete $out->{headers}->{q/"config.h"/};

	my @headers = sort {
	    my ($x, $y) = ($a, $b);
	    foreach ($x, $y) {
		if (/^</) {
		    s/^</!/;
		}
		elsif (!/libgimp/) {
		    s/^/~/;
		}
	    }
	    $x cmp $y;
	} keys %{$out->{headers}};
        my $headers = ""; my $lib = 0; my $seen = 0; my $nl = 0;
	my $sys = 0; my $base = 0;
	foreach (@headers) {
	    $headers .= "\n" if $nl;
	    $nl = 0;

	    if ($_ eq '<unistd.h>') {
		$headers .= "\n" if $seen;
		$headers .= "#ifdef HAVE_UNISTD_H\n";
	    }
	    if ($_ eq '<process.h>') {
		$headers .= "\n" if $seen;
		$headers .= "#include <glib.h>\n\n";	
		$headers .= "#ifdef G_OS_WIN32\n";
	    }


	    $seen++ if /^</;

	    if ($sys == 0 && !/^</) {
		$sys = 1;
		$headers .= "\n";
		$headers .= '#include <glib-object.h>';
		$headers .= "\n\n";
	    }

	    $seen = 0 if !/^</;

	    if (/libgimp/) {
		$lib = 1;
	    }
	    else {
		$headers .= "\n" if $lib;
		$lib = 0;

		if ($sys == 1 && $base == 0) {
		    $base = 1;

		    $headers .= '#include "pdb-types.h"';
		    $headers .= "\n";
		    $headers .= '#include "procedural_db.h"';
		    $headers .= "\n\n";
		}
	    }

            if ($_ eq '"regexrepl/regex.h"') {
		$headers .= "\n";
		$headers .= "#ifdef HAVE_GLIBC_REGEX\n";
		$headers .= "#include <regex.h>\n";
		$headers .= "#else\n";
	    }

	    $headers .= "#include $_\n";

            if ($_ eq '"regexrepl/regex.h"') {
		$headers .= "#endif\n";
		$nl = 1;
	    }

	    if ($_ eq '<unistd.h>') {
		$headers .= "#endif\n";
		$seen = 0;
		$nl = 1;
 	    }

            if ($_ eq '<process.h>') {
		$headers .= "#endif\n";
		$seen = 0;
		$nl = 1;
	    }

	    $headers .= "\n" if $_ eq '"config.h"';
	}

	my $extra = {};
	if (exists $main::grp{$group}->{extra}->{app}) {
	    $extra = $main::grp{$group}->{extra}->{app}
	}

	my $cfile = "$destdir/${group}_cmds.c$FILE_EXT";
	open CFILE, "> $cfile" or die "Can't open $cfile: $!\n";
	print CFILE $gpl;
	print CFILE qq/#include "config.h"\n\n/;
	print CFILE $headers, "\n";
	print CFILE $extra->{decls}, "\n" if exists $extra->{decls};
	print CFILE $out->{procs};
	print CFILE "\nvoid\nregister_${group}_procs (Gimp *gimp)\n";
	print CFILE "{\n$out->{register}}\n";
	print CFILE "\n", $extra->{code} if exists $extra->{code};
	print CFILE $out->{code};
	close CFILE;
	&write_file($cfile);

	my $decl = "register_${group}_procs";
	push @group_decls, $decl;
	$longest = length $decl if $longest < length $decl;

	$group_procs .=  ' ' x 2 . "register_${group}_procs (gimp);\n\n";
	$pcount += $out->{pcount};
    }

    if (! $ENV{PDBGEN_GROUPS}) {
	my $internal = "$destdir/internal_procs.h$FILE_EXT";
	open IFILE, "> $internal" or die "Can't open $internal: $!\n";
	print IFILE $gpl;
	my $guard = "__INTERNAL_PROCS_H__";
	print IFILE <<HEADER;
#ifndef $guard
#define $guard

void internal_procs_init (Gimp *gimp);

#endif /* $guard */
HEADER
	close IFILE;
	&write_file($internal);

	$internal = "$destdir/internal_procs.c$FILE_EXT";
	open IFILE, "> $internal" or die "Can't open $internal: $!\n";
	print IFILE $gpl;
	print IFILE qq@#include "config.h"\n\n@;
	print IFILE qq@#include <glib-object.h>\n\n@;
	print IFILE qq@#include "pdb-types.h"\n\n@;
	print IFILE qq@#include "core/gimp.h"\n\n@;
	print IFILE "/* Forward declarations for registering PDB procs */\n\n";
	foreach (@group_decls) {
	    print IFILE "void $_" . ' ' x ($longest - length $_) . " (Gimp *gimp);\n";
	}
	chop $group_procs;
	print IFILE "\n/* $total procedures registered total */\n\n";
	print IFILE <<BODY;
void
internal_procs_init (Gimp *gimp)
{
  g_return_if_fail (GIMP_IS_GIMP (gimp));

$group_procs
}
BODY
	close IFILE;
	&write_file($internal);
    }
}

1;
