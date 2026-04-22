using System;
using System.Text;

internal static class Program
{
    private static int Main(string[] args)
    {
        string token = null;
        int steps = 0;
        if (!ParseArgs(args, ref token, ref steps))
        {
            Console.Error.WriteLine("Usage: rng-parity-cs --token <replay-token> --steps <N>");
            return 1;
        }

        string normalized = NormalizeReplayToken(token);
        short state = SeedFromToken(token);

        Console.WriteLine("token=" + token);
        Console.WriteLine("normalized=" + normalized);
        Console.WriteLine("seed=" + state);
        for (int n = 0; n < steps; n++)
        {
            int x = 100 + (13 * n);
            int y = 200 + (7 * n);
            int i = 1 + (n % 15);
            uint type = V2Type(state, x, y, i);
            Console.WriteLine(
                "step=" + n +
                " randv=" + state +
                " x=" + x +
                " y=" + y +
                " i=" + i +
                " v2_type=" + type);
            state = LegacyStep(state, x, y);
        }
        return 0;
    }

    private static bool ParseArgs(string[] args, ref string token, ref int steps)
    {
        for (int i = 0; i < args.Length; i++)
        {
            string arg = args[i];
            if (arg == "--token" && i + 1 < args.Length)
            {
                token = args[++i];
                continue;
            }

            if (arg == "--steps" && i + 1 < args.Length)
            {
                if (!int.TryParse(args[++i], out steps))
                {
                    return false;
                }
                continue;
            }

            if (arg == "--help" || arg == "-h")
            {
                return false;
            }
        }

        return !string.IsNullOrEmpty(token) && steps > 0;
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

    private static short LegacyStep(short current, int x, int y)
    {
        short next;
        unchecked
        {
            next = (short)((x * y * current) + 7491);
        }

        if (next == 0)
        {
            unchecked
            {
                next = (short)74917777;
            }
        }

        return next;
    }

    private static uint V2Type(short randv, int x, int y, int i)
    {
        int randv1;
        unchecked
        {
            randv1 = x * y * randv * 7491;
        }

        ushort u16;
        unchecked
        {
            u16 = (ushort)((uint)randv1 * (uint)i);
        }

        return ((uint)u16 * 8u) >> 16;
    }
}
