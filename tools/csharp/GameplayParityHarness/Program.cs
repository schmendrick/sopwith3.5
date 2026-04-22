using System;
using System.Collections.Generic;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;

internal static class Program
{
    private const string ToolId = "gameplay-parity-harness 0.1.0";

    private sealed class TapeRef
    {
        public string LogicalName { get; set; }
        public string Path { get; set; }
    }

    private sealed class MatrixCase
    {
        public string CaseId { get; set; }
        public string Token { get; set; }
        public int Steps { get; set; }
        public List<TapeRef> TapeRefs { get; } = new List<TapeRef>();
    }

    private static int Main(string[] args)
    {
        string caseId = null;
        string matrixVersion = null;
        string token = null;
        int steps = 0;
        string matrixFile = "tools/gameplay-parity-matrix.json";
        if (!ParseArgs(args, ref caseId, ref matrixVersion, ref token, ref steps, ref matrixFile))
        {
            Console.Error.WriteLine(
                "Usage: GameplayParityHarness --case <id> --matrix-version <ver> --token <replay-token> --steps <N> [--matrix-file <path>]");
            return 1;
        }

        if (!File.Exists(matrixFile))
        {
            Console.Error.WriteLine("Failed to read matrix file: " + matrixFile);
            return 2;
        }

        string blob = File.ReadAllText(matrixFile, Encoding.UTF8);
        if (!TryParseMatrix(blob, out string matrixVersionFile, out List<MatrixCase> cases))
        {
            Console.Error.WriteLine("Failed to parse matrix file: " + matrixFile);
            return 2;
        }

        if (matrixVersionFile != matrixVersion)
        {
            Console.Error.WriteLine("matrix_version mismatch (cli=" + matrixVersion + " file=" + matrixVersionFile + ")");
            return 2;
        }

        MatrixCase chosen = null;
        for (int i = 0; i < cases.Count; i++)
        {
            if (cases[i].CaseId == caseId)
            {
                chosen = cases[i];
                break;
            }
        }

        if (chosen == null)
        {
            Console.Error.WriteLine("Unknown case_id: " + caseId);
            return 2;
        }

        if (chosen.Token != token || chosen.Steps != steps)
        {
            Console.Error.WriteLine("CLI token/steps do not match matrix case row");
            return 2;
        }

        RunCase(chosen, matrixVersion);
        return 0;
    }

    private static bool ParseArgs(string[] args, ref string caseId, ref string matrixVersion, ref string token, ref int steps, ref string matrixFile)
    {
        for (int i = 0; i < args.Length; i++)
        {
            string a = args[i];
            if (a == "--case" && i + 1 < args.Length)
            {
                caseId = args[++i];
                continue;
            }

            if (a == "--matrix-version" && i + 1 < args.Length)
            {
                matrixVersion = args[++i];
                continue;
            }

            if (a == "--token" && i + 1 < args.Length)
            {
                token = args[++i];
                continue;
            }

            if (a == "--steps" && i + 1 < args.Length)
            {
                if (!int.TryParse(args[++i], out steps))
                {
                    return false;
                }

                continue;
            }

            if (a == "--matrix-file" && i + 1 < args.Length)
            {
                matrixFile = args[++i];
                continue;
            }

            if (a == "--help" || a == "-h")
            {
                return false;
            }
        }

        return !string.IsNullOrEmpty(caseId) && !string.IsNullOrEmpty(matrixVersion) && !string.IsNullOrEmpty(token) && steps > 0;
    }

    private static bool TryParseMatrix(string blob, out string matrixVersion, out List<MatrixCase> cases)
    {
        matrixVersion = null;
        cases = new List<MatrixCase>();
        try
        {
            using JsonDocument doc = JsonDocument.Parse(blob);
            JsonElement root = doc.RootElement;
            if (!root.TryGetProperty("matrix_version", out JsonElement mvEl) || mvEl.ValueKind != JsonValueKind.String)
            {
                return false;
            }

            matrixVersion = mvEl.GetString();
            if (!root.TryGetProperty("cases", out JsonElement casesEl) || casesEl.ValueKind != JsonValueKind.Array)
            {
                return false;
            }

            foreach (JsonElement c in casesEl.EnumerateArray())
            {
                var mc = new MatrixCase();
                if (!c.TryGetProperty("case_id", out JsonElement idEl) || idEl.ValueKind != JsonValueKind.String)
                {
                    return false;
                }

                if (!c.TryGetProperty("token", out JsonElement tokEl) || tokEl.ValueKind != JsonValueKind.String)
                {
                    return false;
                }

                if (!c.TryGetProperty("steps", out JsonElement stEl) || stEl.ValueKind != JsonValueKind.Number)
                {
                    return false;
                }

                mc.CaseId = idEl.GetString();
                mc.Token = tokEl.GetString();
                mc.Steps = stEl.GetInt32();
                if (c.TryGetProperty("tape_refs", out JsonElement trEl) && trEl.ValueKind == JsonValueKind.Array)
                {
                    foreach (JsonElement t in trEl.EnumerateArray())
                    {
                        if (!t.TryGetProperty("logical_name", out JsonElement ln) || ln.ValueKind != JsonValueKind.String)
                        {
                            return false;
                        }

                        if (!t.TryGetProperty("path", out JsonElement p) || p.ValueKind != JsonValueKind.String)
                        {
                            return false;
                        }

                        mc.TapeRefs.Add(new TapeRef { LogicalName = ln.GetString(), Path = p.GetString() });
                    }
                }

                mc.TapeRefs.Sort((a, b) => string.CompareOrdinal(a.LogicalName, b.LogicalName));
                cases.Add(mc);
            }

            return cases.Count > 0 && !string.IsNullOrEmpty(matrixVersion);
        }
        catch
        {
            return false;
        }
    }

    private static bool EndsWithIgnoreCase(string s, string suffix)
    {
        if (s.Length < suffix.Length)
        {
            return false;
        }

        return s.EndsWith(suffix, StringComparison.OrdinalIgnoreCase);
    }

    private static string NormalizeReplayToken(string replayToken)
    {
        string prefix = string.Empty;
        string basename = replayToken;
        int slash = basename.LastIndexOfAny(new[] { '/', '\\' });
        if (slash >= 0)
        {
            prefix = basename.Substring(0, slash + 1);
            basename = basename.Substring(slash + 1);
        }

        while (true)
        {
            if (EndsWithIgnoreCase(basename, ".tape"))
            {
                basename = basename.Substring(0, basename.Length - 5);
                continue;
            }

            if (EndsWithIgnoreCase(basename, ".rec"))
            {
                basename = basename.Substring(0, basename.Length - 4);
                continue;
            }

            break;
        }

        return prefix + basename + ".tape";
    }

    private static short SeedFromToken(string token)
    {
        string normalized = NormalizeReplayToken(token);
        uint h = 2166136261u;
        byte[] bytes = Encoding.UTF8.GetBytes(normalized);
        for (int i = 0; i < bytes.Length; i++)
        {
            unchecked
            {
                h ^= bytes[i];
                h *= 16777619u;
            }
        }

        short s = (short)(h & 0x7fff);
        if (s == 0)
        {
            s = 1;
        }

        return s;
    }

    private static uint Prng(uint x)
    {
        unchecked
        {
            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 5;
            return x;
        }
    }

    private sealed class MockObj
    {
        public int Id;
        public int XInt;
        public int YInt;
        public int Angle;
        public string Kind;
    }

    private static int MockCompare(MockObj a, MockObj b)
    {
        if (a.XInt != b.XInt)
        {
            return a.XInt < b.XInt ? -1 : 1;
        }

        if (a.YInt != b.YInt)
        {
            return a.YInt < b.YInt ? -1 : 1;
        }

        if (a.Angle != b.Angle)
        {
            return a.Angle < b.Angle ? -1 : 1;
        }

        int k = string.CompareOrdinal(a.Kind, b.Kind);
        if (k != 0)
        {
            return k;
        }

        return a.Id < b.Id ? -1 : (a.Id > b.Id ? 1 : 0);
    }

    private static string MockSortKey(MockObj o)
    {
        return "x=" + o.XInt + "|y=" + o.YInt + "|angle=" + o.Angle + "|kind=" + o.Kind + "|id=" + o.Id;
    }

    private static string ResolveTapePath(string path)
    {
        if (path.IndexOf('/') >= 0 || path.IndexOf('\\') >= 0)
        {
            return path;
        }

        return Path.Combine(".", path);
    }

    private static void EmitHeaders(string matrixVersion, string caseId, string token, string normalized, int steps, List<TapeRef> tapeRefs)
    {
        Console.Out.WriteLine("tool=" + ToolId);
        Console.Out.WriteLine("matrix_version=" + matrixVersion);
        Console.Out.WriteLine("case_id=" + caseId);
        Console.Out.WriteLine("token=" + token);
        Console.Out.WriteLine("normalized=" + normalized);
        Console.Out.WriteLine("steps=" + steps);
        if (tapeRefs.Count == 0)
        {
            Console.Out.WriteLine("tape_ref=none");
            return;
        }

        for (int i = 0; i < tapeRefs.Count; i++)
        {
            string resolved = ResolveTapePath(tapeRefs[i].Path);
            if (!File.Exists(resolved))
            {
                Console.Error.WriteLine("Failed to read tape file for sha256: " + resolved);
                Environment.Exit(2);
            }

            byte[] bytes = File.ReadAllBytes(resolved);
            byte[] hash = SHA256.HashData(bytes);
            string hex = Convert.ToHexString(hash).ToLowerInvariant();
            Console.Out.WriteLine("tape_ref=" + tapeRefs[i].LogicalName + "|path=" + resolved + "|sha256=" + hex);
        }
    }

    private static void RunCase(MatrixCase mc, string matrixVersionCli)
    {
        string normalized = NormalizeReplayToken(mc.Token);
        short seed = SeedFromToken(mc.Token);
        EmitHeaders(matrixVersionCli, mc.CaseId, mc.Token, normalized, mc.Steps, mc.TapeRefs);

        var decisions = new List<string>();
        int steps = mc.Steps;
        int prevCollided = 0;
        int objLife = 3;
        int playerScore = (ushort)seed % 1000;
        int playerLives = 3;
        int enemyScore = (ushort)(seed ^ 0x1357) % 500;
        int enemyLives = 3;

        for (int s = 0; s < steps; s++)
        {
            uint mix = Prng((uint)(ushort)seed ^ (uint)(s * 1315423911));

            if (s == 0)
            {
                decisions.Add("decision logical_step=0 category=spawn_lifecycle key=entity_created entity_id=100 kind=Mock");
                decisions.Add("decision logical_step=0 category=spawn_lifecycle key=entity_created entity_id=101 kind=Mock");
            }

            if (steps > 0 && s == steps - 1)
            {
                decisions.Add("decision logical_step=" + s + " category=spawn_lifecycle key=entity_destroyed entity_id=100 kind=Mock");
            }

            int curCollided = (int)(mix & 1u);
            if (s > 0 && curCollided != prevCollided)
            {
                decisions.Add(
                    "decision logical_step=" + s + " category=collision_outcome key=plane_collided plane=player prev=" + prevCollided +
                    " next=" + curCollided);
            }

            prevCollided = curCollided;

            int prevLife = objLife;
            if (s % 3 == 0 && objLife > 0)
            {
                objLife -= 1;
            }

            if (prevLife > 0 && objLife <= 0)
            {
                decisions.Add(
                    "decision logical_step=" + s + " category=collision_outcome key=object_life entity_id=200 prev_life=" + prevLife +
                    " new_life=" + objLife + " crossed=1");
            }

            int prevPs = playerScore;
            int prevPl = playerLives;
            int prevEs = enemyScore;
            int prevEl = enemyLives;

            if ((mix % 4u) == 0u)
            {
                playerScore += (int)(mix % 5u) - 2;
            }

            if (s > 0 && (s % 11) == 0 && playerLives > 0)
            {
                playerLives -= 1;
            }

            if ((mix % 6u) == 0u)
            {
                enemyScore += (int)((mix >> 3) % 3u) - 1;
            }

            if (s > 0 && (s % 13) == 0 && enemyLives > 0)
            {
                enemyLives -= 1;
            }

            int dps = playerScore - prevPs;
            int dpl = playerLives - prevPl;
            if (dps != 0 || dpl != 0)
            {
                decisions.Add(
                    "decision logical_step=" + s + " category=score_lives_delta key=player score_delta=" + dps + " lives_delta=" + dpl);
            }

            int des = enemyScore - prevEs;
            int del = enemyLives - prevEl;
            if (des != 0 || del != 0)
            {
                decisions.Add(
                    "decision logical_step=" + s + " category=score_lives_delta key=enemy score_delta=" + des + " lives_delta=" + del);
            }

            var o1 = new MockObj();
            o1.Id = 1;
            o1.XInt = 10 + ((int)mix % 50);
            o1.YInt = 20 + (((int)mix >> 3) % 40);
            o1.Angle = (s * 7) % 360;
            o1.Kind = "Bomb";
            var o2 = new MockObj();
            o2.Id = 2;
            o2.XInt = o1.XInt;
            o2.YInt = o1.YInt + 1 + ((int)mix % 3);
            o2.Angle = (s * 5) % 360;
            o2.Kind = "Bird";
            var objs = new List<MockObj> { o1, o2 };
            objs.Sort(MockCompare);
            var keyBuf = new StringBuilder();
            for (int i = 0; i < objs.Count; i++)
            {
                if (i > 0)
                {
                    keyBuf.Append(';');
                }

                keyBuf.Append(MockSortKey(objs[i]));
            }

            decisions.Add(
                "decision logical_step=" + s + " category=replay_entity_order key=object_sort_keys keys=" + keyBuf.ToString());
            decisions.Add(
                "decision logical_step=" + s + " category=replay_entity_order key=entity_row_order rows=PLAYER|ENEMY|OBJECT|OBJECT");
        }

        decisions.Sort(StringComparer.Ordinal);
        for (int i = 0; i < decisions.Count; i++)
        {
            Console.Out.WriteLine(decisions[i]);
        }
    }
}
