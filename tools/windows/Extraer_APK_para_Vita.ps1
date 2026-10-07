#requires -Version 5.1
[CmdletBinding()]
param(
    [string[]]$ApkPaths = @(),
    [string]$OutputRoot = '',
    [switch]$FromLauncher,
    [switch]$NoOpen
)
Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'
$utf8 = New-Object System.Text.UTF8Encoding($false)
$stage = $null
$final = $null

$KnownOriginalSha = 'b84f98a3ed70957354f358b7930bd8fb651cc89b74e16f8774ebd989fbf0899b'
$KnownGenSha = 'd52cbd7ef248d995ad17ba6ec8ec6fa08590a344ac2a9786e5ac839bf7715f28'
$KnownSamuSha = '1771d71de25d664894dfb33b4a296ad6d30f5d897a34eb1ec14f3135496ec41d'
$KnownAndroid14Sha = 'a210795bf7ded8636a91bea96df051557229149feb310cf07baf16b0731e79c4'
$KnownSpanishSha = 'b38cc2c4ae3f20d1b1c6c1419a7b6b62ab57ea8954468874f6c5f8c40b39a098'
$KnownInvasionSha = 'caaf294ddb9bf833868d7b541fc310603827bed44072230f60e0552cbb2dc94d'

# Use only Windows/.NET built-ins. This helper checks ZIP CRCs explicitly:
# ZipArchive on .NET Framework does not guarantee CRC verification on read.
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
Add-Type -TypeDefinition @'
using System;
using System.IO;
using System.Collections.Generic;
public sealed class DbtbZipRecord {
    public uint Crc, Size, CompressedSize, Attributes;
    public ushort Flags, Method;
}
public static class DbtbZipChecks {
    static readonly uint[] Table = MakeTable();
    static uint[] MakeTable() {
        var t = new uint[256];
        for (uint i = 0; i < 256; i++) {
            uint x = i;
            for (int k = 0; k < 8; k++) x = (x & 1) != 0 ? 0xedb88320U ^ (x >> 1) : x >> 1;
            t[i] = x;
        }
        return t;
    }
    public static DbtbZipRecord[] Central(Stream stream) {
        if (stream.Length < 22 || stream.Length > 1024L * 1024 * 1024)
            throw new InvalidDataException("Truncated APK or APK larger than 1 GiB.");
        int n = (int)Math.Min(stream.Length, 65557);
        byte[] tail = new byte[n];
        stream.Position = stream.Length - n;
        int got = 0;
        while (got < n) {
            int r = stream.Read(tail, got, n - got);
            if (r == 0) throw new EndOfStreamException();
            got += r;
        }
        int e = -1;
        for (int i = n - 22; i >= 0; i--) {
            if (BitConverter.ToUInt32(tail, i) == 0x06054b50U &&
                i + 22 + BitConverter.ToUInt16(tail, i + 20) == n) { e = i; break; }
        }
        if (e < 0) throw new InvalidDataException("Not a valid APK/ZIP archive.");
        ushort count = BitConverter.ToUInt16(tail, e + 10);
        uint size = BitConverter.ToUInt32(tail, e + 12);
        uint offset = BitConverter.ToUInt32(tail, e + 16);
        if (BitConverter.ToUInt16(tail, e + 4) != 0 || BitConverter.ToUInt16(tail, e + 6) != 0 ||
            BitConverter.ToUInt16(tail, e + 8) != count || count == 65535 ||
            count > 8192 || size > 16 * 1024 * 1024 ||
            (long)offset + size > stream.Length - n + e)
            throw new InvalidDataException("Split ZIP, ZIP64, or out-of-bounds directory is not supported.");
        stream.Position = offset;
        var br = new BinaryReader(stream, System.Text.Encoding.UTF8, true);
        var result = new List<DbtbZipRecord>();
        for (int i = 0; i < count; i++) {
            if (br.ReadUInt32() != 0x02014b50U) throw new InvalidDataException("Damaged ZIP central directory.");
            br.ReadUInt16(); br.ReadUInt16();
            var r = new DbtbZipRecord();
            r.Flags = br.ReadUInt16(); r.Method = br.ReadUInt16();
            br.ReadUInt16(); br.ReadUInt16();
            r.Crc = br.ReadUInt32(); r.CompressedSize = br.ReadUInt32(); r.Size = br.ReadUInt32();
            ushort name = br.ReadUInt16(), extra = br.ReadUInt16(), comment = br.ReadUInt16();
            ushort disk = br.ReadUInt16(); br.ReadUInt16();
            r.Attributes = br.ReadUInt32(); uint local = br.ReadUInt32();
            if (disk != 0 || r.Size == uint.MaxValue || r.CompressedSize == uint.MaxValue ||
                local == uint.MaxValue || local >= offset)
                throw new InvalidDataException("Unsupported or out-of-bounds ZIP entry.");
            stream.Seek(name + extra + comment, SeekOrigin.Current);
            if (stream.Position > (long)offset + size) throw new InvalidDataException("Truncated ZIP directory.");
            result.Add(r);
        }
        if (stream.Position != (long)offset + size) throw new InvalidDataException("Incorrect ZIP directory size.");
        stream.Position = 0;
        return result.ToArray();
    }
    public static void CopyChecked(Stream source, string target, long expected, uint expectedCrc) {
        byte[] buffer = new byte[65536]; uint crc = 0xffffffffU; long total = 0;
        using (var dest = new FileStream(target, FileMode.CreateNew, FileAccess.Write, FileShare.None)) {
            int read;
            while ((read = source.Read(buffer, 0, buffer.Length)) > 0) {
                total += read;
                if (total > expected) throw new InvalidDataException("Decompressed data exceeds the declared size.");
                for (int i = 0; i < read; i++) crc = Table[(crc ^ buffer[i]) & 255] ^ (crc >> 8);
                dest.Write(buffer, 0, read);
            }
        }
        if (total != expected || (crc ^ 0xffffffffU) != expectedCrc)
            throw new InvalidDataException("Damaged APK: size or CRC check failed.");
    }
}
'@

function Get-CommunityProfiles {
    $legacy = [pscustomobject]@{
        Name='community14-a210795b'; Count=[uint32]42802; Offset=[uint32]996678763; Size=[uint32]47633006;
        Types=@([uint32]2566558864,[uint32]1246424290,[uint32]1133921820,[uint32]1102453101,[uint32]1111565669,[uint32]1184565109,[uint32]2268849933);
        Fixed=@{ '2752'='common'; '1BC2'='select0'; '9B28'='effect'; '59F2'='demo_00'; '3C90'='demo_08'; 'D0BD'='font00'; '5D73'='card_preview'; 'D67E'='gamedata'; '82B7'='text00' };
        Numbered=@{ '0B49'=@('back',2); 'BDC7'=@('bobj',2); 'E03B'=@('char',2); '8AC1'=@('chardemo',2); 'FAFD'=@('charf',4); '47DD'=@('card',3) }
    }
    $spanish = [pscustomobject]@{
        Name='community14-es-d594affc'; Count=[uint32]59050; Offset=[uint32]830950436; Size=[uint32]2030988207;
        Types=@([uint32]16535934,[uint32]3988003272,[uint32]3567234535,[uint32]1591802006,[uint32]63084285,[uint32]1538081446);
        Fixed=@{ '4D7F'='common'; 'B4EB'='select0'; 'B248'='effect'; 'AC3B'='demo_00'; '8827'='demo_08'; '4919'='card_preview'; 'EC5A'='gamedata'; 'A602'='text00' };
        Numbered=@{ '0294'=@('back',2); 'D794'=@('bobj',2); 'F298'=@('char',2); 'AE52'=@('chardemo',2); 'EB21'=@('charf',4); '6FA6'=@('card',3) }
    }
    $invasion = [pscustomobject]@{
        Name='community14-invasion-05aa0c5e'; Count=[uint32]33839; Offset=[uint32]1901542107; Size=[uint32]866934865;
        Types=@([uint32]2931803040,[uint32]2273195935,[uint32]2166741075,[uint32]3888254667,[uint32]1077762279,[uint32]356305721);
        Fixed=@{ '9036'='common'; '7E8F'='select0'; '1E1C'='effect'; '97E6'='demo_00'; '6E24'='demo_08'; '0708'='card_preview'; '90EA'='gamedata'; 'D37C'='text00' };
        Numbered=@{ 'F813'=@('back',2); '17A5'=@('bobj',2); '0953'=@('char',2); '364E'=@('chardemo',2); '91F9'=@('charf',4); '1A4B'=@('card',3) }
    }
    return @($legacy,$spanish,$invasion)
}

function Get-CanonicalName([string]$Name, [string]$Codec = '') {
    if ($Name.Contains('/') -or -not $Name.EndsWith('.pac', [StringComparison]::Ordinal)) { return $Name }
    $profiles = @(Get-CommunityProfiles)
    if ($Codec) { $profiles = @($profiles | Where-Object { $_.Name -ceq $Codec }) }
    $stem = $Name.Substring(0, $Name.Length - 4)
    $canonicalResults = @()
    foreach ($profile in $profiles) {
        if ($profile.Fixed.ContainsKey($stem)) {
            $canonicalResults += $profile.Fixed[$stem] + '.pac'
            continue
        }
        foreach ($prefix in $profile.Numbered.Keys) {
            $spec = $profile.Numbered[$prefix]
            $digits = [int]$spec[1]
            $pattern = '^' + $prefix + '([0-9]{' + $digits + '})$'
            if ($stem -cmatch $pattern) {
                $canonicalResults += [string]$spec[0] + $Matches[1] + '.pac'
                break
            }
        }
    }
    $unique = @($canonicalResults | Select-Object -Unique)
    if ($unique.Count -eq 1) { return $unique[0] }
    return $Name
}

function Get-CommunityProfileFromNames($Names) {
    $best = $null; $bestScore = 0; $tie = $false
    foreach ($profile in @(Get-CommunityProfiles)) {
        $score = 0
        foreach ($name in $Names) { if ((Get-CanonicalName $name $profile.Name) -cne $name) { $score++ } }
        if ($score -gt $bestScore) { $best=$profile; $bestScore=$score; $tie=$false }
        elseif ($score -gt 0 -and $score -eq $bestScore) { $tie=$true }
    }
    if ($tie -or -not $best) { return $null }
    return $best.Name
}

function Assert-SafeName([string]$Name) {
    if ([string]::IsNullOrEmpty($Name) -or $Name -match '[\x00-\x1f\x7f\\:*?"<>|]' -or
        $utf8.GetByteCount($Name) -gt 200) { throw "Unsafe or overly long path: $Name" }
    foreach ($part in $Name.Split('/')) {
        if ($part -eq '' -or $part -eq '.' -or $part -eq '..' -or $part -match '[. ]$' -or
            $part -match '^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])([.]|$)' -or
            $part -match '^(dbtb_manifest[.]json|mod[.]json)([.]|$)') {
            throw "Unsafe or reserved name: $Name"
        }
    }
}

function Assert-CommunityPac([string]$Path, [string]$Codec) {
    $profile = @(Get-CommunityProfiles) | Where-Object { $_.Name -ceq $Codec } | Select-Object -First 1
    if (-not $profile) { throw "Unknown Android14 profile: $Codec" }
    $bytes = [IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -lt 18) { throw "Truncated Android14 PAC: $Path" }
    $count = [BitConverter]::ToUInt16($bytes, 0) -bxor $profile.Count
    $base = 2L + $count * 16L
    if ($count -eq 0 -or $base -gt $bytes.Length) { throw "Unsupported Android14 PAC codec: $Path" }
    $found = $false
    for ($i = 0; $i -lt $count; $i++) {
        $pos = 2 + $i * 16
        $offset = [BitConverter]::ToUInt32($bytes, $pos) -bxor $profile.Offset -bxor [uint32]$i
        $size = [BitConverter]::ToUInt32($bytes, $pos + 4) -bxor $profile.Size -bxor [uint32]$i
        [uint32]$key = ([uint32]$bytes[$pos+8] * 16777216L + [uint32]$bytes[$pos+9] * 65536L +
                        [uint32]$bytes[$pos+10] * 256L + [uint32]$bytes[$pos+11])
        $key = $key -bxor [uint32]$i
        if ($profile.Types -contains $key) { $found = $true }
        if ($base + [long]$offset -gt $bytes.Length -or [long]$size -gt $bytes.Length - $base - [long]$offset) {
            throw "Android14 PAC entry is out of bounds: $Path"
        }
    }
    if (-not $found) { throw "Unknown Android14 PAC codec: $Path" }
}

function Get-Sha([string]$Path) { return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant() }
function Write-Json([string]$Path, $Value) {
    [IO.File]::WriteAllText($Path, ($Value | ConvertTo-Json -Depth 12) + "`n", $utf8)
}
function Get-ProfileName([string]$Stem) {
    $name = [regex]::Replace($Stem, '[^a-zA-Z0-9_-]', '_').Trim('_')
    if ($name.Length -gt 36) { $name = $name.Substring(0,36) }
    if ($name -eq '') { $name = 'Mod' }
    if ($name -match '^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])$') { $name = 'Mod_' + $name }
    return $name
}
function Assert-NoReparse([string]$Path) {
    $current = [IO.Path]::GetFullPath($Path)
    while ($current) {
        if ([IO.File]::Exists($current) -or [IO.Directory]::Exists($current)) {
            if (([IO.File]::GetAttributes($current) -band [IO.FileAttributes]::ReparsePoint) -ne 0) {
                throw "Output path cannot contain symlinks or junctions: $current"
            }
        }
        $current = [IO.Path]::GetDirectoryName($current)
    }
}

function Get-CharacterInventory($Names) {
    $complete = 0
    $incomplete = @()
    $gapSeen = $false
    $laterAfterGap = @()

    for ($i = 0; $i -lt 100; $i++) {
        $triplet = @(
            ('char{0:D2}.pac' -f $i),
            ('chardemo{0:D2}.pac' -f $i),
            ('charf{0:D4}.pac' -f $i)
        )
        $present = @($triplet | Where-Object { $Names.ContainsKey($_) })
        if ($present.Count -eq 0) {
            $gapSeen = $true
            continue
        }
        if ($present.Count -ne 3) { $incomplete += $i; continue }
        if ($gapSeen) { $laterAfterGap += $i; continue }
        $complete++
    }

    $unsupported = @($Names.Keys | Where-Object {
        ($_ -match '^char[0-9]{3,}[.]pac\z') -or
        ($_ -match '^chardemo[0-9]{3,}[.]pac\z') -or
        ($_ -match '^charf[0-9]{5,}[.]pac\z')
    } | Sort-Object)

    $completeIndices = ''
    if ($complete -gt 0) { $completeIndices = ('00..{0:D2}' -f ($complete - 1)) }
    return [pscustomobject]@{
        Count=$complete
        CompleteIndices=$completeIndices
        Incomplete=@($incomplete)
        LaterAfterGap=@($laterAfterGap)
        Unsupported=@($unsupported)
        RuntimeCompatible=($complete -ge 13 -and $incomplete.Count -eq 0 -and $laterAfterGap.Count -eq 0 -and $unsupported.Count -eq 0)
    }
}

function Import-Apk([string]$Apk, [string]$Package, $UsedProfiles) {
    $stream = $null; $archive = $null
    try {
        if (-not [IO.File]::Exists($Apk) -or [IO.Path]::GetExtension($Apk) -ine '.apk') {
            throw "Not an existing APK file: $Apk"
        }
        $stream = [IO.File]::Open($Apk, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::Read)
        $records = [DbtbZipChecks]::Central($stream)
        $sha = [Security.Cryptography.SHA256]::Create()
        try { $apkHash = ([BitConverter]::ToString($sha.ComputeHash($stream))).Replace('-','').ToLowerInvariant() }
        finally { $sha.Dispose(); $stream.Position = 0 }
        $archive = New-Object IO.Compression.ZipArchive($stream, [IO.Compression.ZipArchiveMode]::Read, $true)
        if ($archive.Entries.Count -ne $records.Length) { throw 'ZIP directory metadata does not match.' }
        $raw = @($archive.Entries | Where-Object { $_.FullName.StartsWith('res/raw/',[StringComparison]::Ordinal) -and $_.Name -ne '' })
        $assets = @($archive.Entries | Where-Object { $_.FullName.StartsWith('assets/',[StringComparison]::Ordinal) -and $_.Name -ne '' })
        [long]$rawBytes = 0; [long]$assetBytes = 0
        foreach ($entry in $raw) { $rawBytes += $entry.Length }
        foreach ($entry in $assets) { $assetBytes += $entry.Length }
        if ($rawBytes -gt 0 -and $assetBytes -gt 0) { throw 'Ambiguous APK: non-empty game data exists in both res/raw and assets.' }
        if ($rawBytes -gt 0) {
            $layout = 'raw'; $prefix = 'res/raw/'
        }
        elseif ($assetBytes -gt 0) {
            $layout = 'assets'; $prefix = 'assets/'; $communityCodec = $null
            $communityCodec = Get-CommunityProfileFromNames @($assets | ForEach-Object { $_.FullName.Substring(7) })
            if ($communityCodec) { $layout = 'community14' }
        }
        else { throw 'The APK contains no usable game data in res/raw or assets.' }

        # Every APK is a standalone Vita profile. The profile folder name is
        # always derived from the APK filename; codec/layout detection never
        # changes the visible profile name.
        $profile = 'profiles/' + (Get-ProfileName ([IO.Path]::GetFileNameWithoutExtension($Apk)))
        $originalProfile = $profile; $suffix = 2
        while ($UsedProfiles.ContainsKey($profile.ToLowerInvariant())) {
            $profile = $originalProfile + '_' + $suffix
            $suffix++
        }
        $UsedProfiles.Add($profile.ToLowerInvariant(), $true)
        $target = Join-Path (Join-Path $Package 'data/DBTapBattle') $profile
        $items = New-Object 'System.Collections.Generic.List[object]'
        $names = @{}; $bundledSave = $false; [long]$total = 0
        for ($index = 0; $index -lt $archive.Entries.Count; $index++) {
            $entry = $archive.Entries[$index]
            if (-not $entry.FullName.StartsWith($prefix,[StringComparison]::Ordinal)) { continue }
            $name = $entry.FullName.Substring($prefix.Length)
            if ($entry.Name -eq '') { if ($name) { Assert-SafeName $name.TrimEnd('/') }; continue }
            Assert-SafeName $name
            if ($layout -eq 'community14') { $name = Get-CanonicalName $name $communityCodec }
            $key = $name.ToLowerInvariant()
            if ($names.ContainsKey($key)) { throw "Duplicate name after normalization: $name" }
            $record = $records[$index]
            $mode = ($record.Attributes -shr 16) -band 61440
            if ($mode -ne 0 -and $mode -ne 32768) { throw "Non-regular ZIP entry: $name" }
            if (($record.Flags -band 1) -ne 0 -or $record.Method -notin @(0,8)) { throw "Encrypted entry or unsupported compression: $name" }
            if ($entry.Length -ne $record.Size) { throw "Inconsistent ZIP size: $name" }
            if ($key -eq 'save.bin') {
                if ($bundledSave) { throw 'The selected APK layout contains more than one save.bin.' }
                $bundledSave = $true
                continue
            }
            $names.Add($key,$true)
            $total += $entry.Length
            if ($entry.Length -gt 64MB -or $total -gt 512MB) { throw 'Data exceeds limits: 64 MiB per file / 512 MiB per APK.' }
            $items.Add([pscustomobject]@{ Entry=$entry; Name=$name; Record=$record })
        }
        foreach ($key in $names.Keys) {
            $parts = $key.Split('/')
            for ($i=1; $i -lt $parts.Length; $i++) {
                if ($names.ContainsKey(($parts[0..($i-1)] -join '/'))) { throw "File/directory collision: $key" }
            }
        }
        if (-not $names.ContainsKey('common.pac')) { throw "common.pac was not found; this does not look like a complete Tap Battle APK: $Apk" }
        [void][IO.Directory]::CreateDirectory($target)
        $files = @(); $renamed = @(); $unknown = @()
        $known = @('.pac','.ogg','.png','.bmp','.bin','.dat','.db','.dac','.gdt','.cnv','.spr','.act','.plt','.xml')
        $position = 0
        foreach ($item in $items) {
            $position++
            Write-Progress -Activity ([IO.Path]::GetFileName($Apk)) -Status $item.Name -PercentComplete ($position*100/$items.Count)
            $file = Join-Path $target $item.Name
            [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($file))
            $source = $item.Entry.Open()
            try { [DbtbZipChecks]::CopyChecked($source, $file, $item.Entry.Length, $item.Record.Crc) }
            finally { $source.Dispose() }
            if ($layout -eq 'community14' -and $item.Name.EndsWith('.pac',[StringComparison]::Ordinal)) { Assert-CommunityPac $file $communityCodec }
            $files += [pscustomobject]@{ name=$item.Name; size=$item.Entry.Length; sha256=(Get-Sha $file); apk_path=$item.Entry.FullName }
            if ($item.Name -cne $item.Entry.FullName.Substring($prefix.Length)) {
                $renamed += [pscustomobject]@{ apk_path=$item.Entry.FullName; name=$item.Name }
            }
            if ([IO.Path]::GetExtension($item.Name).ToLowerInvariant() -notin $known) { $unknown += $item.Name }
        }
        Write-Progress -Activity ([IO.Path]::GetFileName($Apk)) -Completed
        $codec = 'original-or-unknown'
        if ($layout -eq 'community14') { $codec = $communityCodec }
        $roster = Get-CharacterInventory $names
        $rawUnknown = @(); if ($layout -eq 'raw') { $rawUnknown = $unknown }
        $manifest = [ordered]@{ format=4; tool='DBTapBattle Windows Extractor 1.4'; source_layout=$layout;
            pac_codec=$codec; payloads_unchanged=$true; standalone_profile=$true;
            requires_game_directory=$false; renamed_files=@($renamed);
            source_apk=[IO.Path]::GetFileName($Apk); source_apk_sha256=$apkHash;
            file_count=$files.Count; files=@($files); unknown_files=@($unknown); unknown_raw_files=@($rawUnknown);
            not_extracted=@($archive.Entries | Where-Object { -not $_.FullName.StartsWith($prefix,[StringComparison]::Ordinal) -and $_.Name -ne '' } | ForEach-Object { $_.FullName });
            vita_profile=$profile; character_count=$roster.Count; character_indices=$roster.CompleteIndices;
            character_runtime_compatible=$roster.RuntimeCompatible;
            incomplete_character_indices=@($roster.Incomplete);
            character_indices_after_gap=@($roster.LaterAfterGap);
            unsupported_character_files=@($roster.Unsupported);
            save_policy='per-profile-vpk-seed';
            bundled_save=$bundledSave; profile_save_installed=$false }
        Write-Json (Join-Path $target 'dbtb_manifest.json') $manifest
        Write-Host ("OK: {0} -> {1} ({2} files, {3} renamed, {4} characters)" -f [IO.Path]::GetFileName($Apk),$profile,$files.Count,$renamed.Count,$roster.Count)
        if (-not $roster.RuntimeCompatible) {
            Write-Host 'WARNING: the roster does not fully satisfy the Vita 00.33+ contract (contiguous 00..99 namespace, no partial character triplets).' -ForegroundColor Yellow
        }
        if ($roster.Unsupported.Count) {
            Write-Host 'WARNING: 3+ digit character IDs were found; the current Vita runtime supports indices 00..99.' -ForegroundColor Yellow
        }
        if ($bundledSave) { Write-Host 'INFO: the APK contains save.bin, but the extractor does not install it; the VPK creates an independent save inside each profile.' -ForegroundColor Yellow }
        return [pscustomobject]@{ apk=[IO.Path]::GetFileName($Apk); profile=$profile; files=$files.Count;
            source_sha256=$apkHash; character_count=$roster.Count; character_indices=$roster.CompleteIndices;
            character_runtime_compatible=$roster.RuntimeCompatible; bundled_save=$bundledSave }
    }
    finally {
        if ($archive) { $archive.Dispose() }
        if ($stream) { $stream.Dispose() }
    }
}

try {
    Write-Host 'DRAGON BALL TAP BATTLE - PS VITA DATA EXTRACTOR' -ForegroundColor Cyan
    if ($FromLauncher) {
        if ([string]::IsNullOrEmpty($OutputRoot)) { $OutputRoot = $env:DBTB_OUTPUT }
        $ApkPaths = @()
        for ($i=0; $i -lt [int]$env:DBTB_ARG_COUNT; $i++) {
            $ApkPaths += [Environment]::GetEnvironmentVariable('DBTB_APK_' + $i)
        }
    }
    if ($ApkPaths.Count -eq 0) {
        Add-Type -AssemblyName System.Windows.Forms
        $dialog = New-Object Windows.Forms.OpenFileDialog
        try {
            $dialog.Title = 'Choose one or more Dragon Ball Tap Battle APKs'
            $dialog.Filter = 'APK files (*.apk)|*.apk'
            $dialog.Multiselect = $true
            if ($dialog.ShowDialog() -ne [Windows.Forms.DialogResult]::OK) { Write-Host 'Cancelled.'; exit 0 }
            $ApkPaths = @($dialog.FileNames)
        } finally { $dialog.Dispose() }
    }
    if ([string]::IsNullOrEmpty($OutputRoot)) { $OutputRoot = Join-Path $PSScriptRoot 'Listo_para_Vita' }
    $OutputRoot = [IO.Path]::GetFullPath($OutputRoot)
    Assert-NoReparse $OutputRoot
    [void][IO.Directory]::CreateDirectory($OutputRoot)
    $id = (Get-Date -Format 'yyyyMMdd_HHmmss') + '_' + [Guid]::NewGuid().ToString('N').Substring(0,8)
    $stage = Join-Path $OutputRoot ('.extrayendo_' + $id)
    $final = Join-Path $OutputRoot ('Paquete_' + $id)
    [void][IO.Directory]::CreateDirectory($stage)
    $used = @{}; $reports = @()
    $unique = @{}
    foreach ($apk in $ApkPaths) {
        $full = [IO.Path]::GetFullPath($apk)
        if ($unique.ContainsKey($full.ToLowerInvariant())) { continue }
        $unique.Add($full.ToLowerInvariant(),$true)
        $reports += Import-Apk $full $stage $used
    }
    $lines = @('DRAGON BALL TAP BATTLE DATA FOR PS VITA', '',
        'Copy the data folder from THIS package to the ux0: root using VitaShell.',
        'The final path must be ux0:data/DBTapBattle/profiles/<Profile>/. Do not copy the Package_* folder itself into ux0:data.',
        'Install the Dragon Ball Tap Battle Vita VPK separately. This package contains data only.', '', 'EXTRACTED PROFILES:')
    foreach ($report in $reports) {
        $label = $report.profile.Substring($report.profile.LastIndexOf('/')+1)
        if ($report.profile -eq 'game') { $label = 'Original' }
        $lines += ('- {0}: ux0:data/DBTapBattle/{1}/ -> select {2} in the VPK.' -f $report.apk,$report.profile,$label)
        $lines += ('  Characters detected: {0} {1}' -f $report.character_count,$report.character_indices)
        if (-not $report.character_runtime_compatible) { $lines += '  WARNING: the roster does not fully satisfy the current Vita runtime contract; check dbtb_manifest.json.' }
        if ($report.bundled_save) { $lines += '  The APK contained save.bin, but it is not installed: the Vita port creates an independent profile save from the VPK seed.' }
    }
    $lines += @('',
        'Every APK is stored as an independent profile under profiles/, using the APK filename as the profile folder.',
        'You can install any supported complete APK as its own profile; there is no separate game/ directory.',
        'The current runtime does NOT fall back between profiles. If a resource is missing, that APK/mod needs compatibility work; do not copy it from another profile.',
        'To change the name shown in the Vita selector, rename the extracted folder inside profiles/.',
        'The folder name is the selector name; no other metadata rename is required.', '',
        'IMPORTANT WHEN UPDATING AN EXISTING INSTALLATION:',
        'Each profile uses its own save.bin inside profiles/<Profile>/.',
        'The VPK creates that save from its bundled seed only when the profile save does not already exist.',
        'save.bin files bundled inside APKs/mods are not copied automatically; each profile starts from the same VPK seed.',
        'Do not share save files between Original, Gen, Android14, or other profiles.',
        'Each run creates a new package and does not delete or merge previous outputs.', '',
        'PAC and media files are preserved byte-for-byte. Protected Android14-family profiles only rename audited aliases.',
        'File sizes, ZIP CRCs, and SHA-256 hashes are verified. dbtb_manifest.json records provenance.',
        'APK files, DEX code, and Android libraries are not included. Mods that change Android code',
        'may require port changes; extracting data does not reproduce Android code changes.',
        'RESULTADO.json and SHA256SUMS.txt document this package.')
    [IO.File]::WriteAllLines((Join-Path $stage 'LEEME_COPIAR_A_VITA.txt'), [string[]]$lines, $utf8)
    Write-Json (Join-Path $stage 'RESULTADO.json') ([ordered]@{ tool_version='1.4'; standalone_profiles=$true; verified=$true; profiles=@($reports) })
    $sums = @()
    foreach ($file in @(Get-ChildItem -LiteralPath $stage -Recurse -File | Sort-Object FullName)) {
        $relative = $file.FullName.Substring($stage.Length+1).Replace('\','/')
        $sums += (Get-Sha $file.FullName) + '  ' + $relative
    }
    [IO.File]::WriteAllLines((Join-Path $stage 'SHA256SUMS.txt'), [string[]]$sums, $utf8)
    [IO.Directory]::Move($stage, $final)
    $stage = $null
    Write-Host ''
    Write-Host 'READY. Copy the data folder from:' -ForegroundColor Green
    Write-Host $final
    Write-Host 'to the ux0: root of your PS Vita. Read LEEME_COPIAR_A_VITA.txt before replacing any existing profile data.'
    if ($FromLauncher -and -not $NoOpen -and $env:DBTB_NO_OPEN -ne '1') {
        try { Invoke-Item -LiteralPath $final } catch { Write-Host 'Explorer could not be opened; the package folder was still created.' }
    }
    exit 0
}
catch {
    if ($stage -and [IO.Directory]::Exists($stage)) { [IO.Directory]::Delete($stage, $true) }
    Write-Host ('ERROR: ' + $_.Exception.Message) -ForegroundColor Red
    Write-Host 'No partial package was published. Your APKs and previous outputs are unchanged.'
    exit 2
}
