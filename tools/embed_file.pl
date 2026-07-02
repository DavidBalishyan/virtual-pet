# Pre-build script: converts text files into a C header with PROGMEM strings.
# Usage: perl tools/embed_file.pl <dst> <src1>:<var1> [<src2>:<var2> ...]
use strict;
use warnings;
use autodie;

my $dst = shift @ARGV;
my @pairs = @ARGV;

if (!$dst || !@pairs) {
    die "Usage: $0 <dst> <src1>:<var1> [<src2>:<var2> ...]\n";
}

my $header = "// Auto-generated. Do not edit manually.\n";
$header   .= "#ifndef DASHBOARD_ASSETS_H\n#define DASHBOARD_ASSETS_H\n\n";
$header   .= "#include <Arduino.h>\n";

my $total = 0;
for my $pair (@pairs) {
    my ($src, $varname) = split ':', $pair, 2;
    if (!$src || !$varname) {
        die "Invalid pair '$pair'. Expected src:varname\n";
    }
    if (!-e $src) {
        warn "[embed_file] WARNING: $src not found, skipping\n";
        next;
    }
    open my $fh, '<', $src;
    my $content = do { local $/; <$fh> };
    close $fh;

    $header .= "\nstatic const char ${varname}[] PROGMEM = R\"rawliteral(${content})rawliteral\";\n";
    $total  += length($content);
    printf "[embed_file] Inline %-30s -> %-25s (%4d bytes)\n", $src, $varname, length($content);
}

$header .= "\n#endif // DASHBOARD_ASSETS_H\n";

open my $out, '>', $dst;
print $out $header;
close $out;

printf "[embed_file] Wrote %s (%d bytes total)\n", $dst, $total;
