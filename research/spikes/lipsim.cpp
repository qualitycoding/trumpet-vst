// Spike: one-mass outward-striking lip valve coupled to a modal input impedance (sum of second-order modes).
// Build: g++ -O2 -std=c++20 -o lipsim lipsim.cpp
// Usage: lipsim <modes.txt> key=value ...   (see defaults in Params)
//   modes.txt: first line Zc (Pa s/m^3, input characteristic impedance), then lines "f_Hz Q A" (A = peak |Z|/Zc).
//   track=1 prints one JSON line per 25 ms window (f0, rms) instead of a single steady-state summary.
//   wav=<path> writes the mouthpiece pressure (normalised, float32 mono, at fs/os).
//
// Lip equation (outward striking, e.g. Velut et al. 2017; Fletcher 1993):
//   y'' + (wl/Ql) y' + wl^2 (y - H) = (pm - p) / mu,      h = max(y, 0)
// Flow (Bernoulli, quasi-static):  u = b h sign(pm - p) sqrt(2 |pm - p| / rho)
// Bore: p = sum_n Z_n(s) u,  Z_n(s) = Zc A_n (wn/Qn) s / (s^2 + (wn/Qn) s + wn^2), split into complex pole pairs and
// discretised with the bilinear transform at fs*os; the flow/pressure coupling is solved implicitly in closed form.
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <string>
#include <vector>

using cd = std::complex<double>;

struct Mode { double f, Q, A; };

static double arg(std::map<std::string, std::string>& kv, const char* k, double d) {
    auto it = kv.find(k); return it == kv.end() ? d : std::atof(it->second.c_str());
}

// Normalised autocorrelation f0 over x (returns 0 if aperiodic). Search range [fmin, fmax].
static double f0_acf(const std::vector<double>& x, double fs, double fmin, double fmax, double* clarity) {
    const size_t n = x.size();
    double mean = 0; for (double v : x) mean += v; mean /= double(n);
    std::vector<double> y(n); for (size_t i = 0; i < n; ++i) y[i] = x[i] - mean;
    const int lmin = int(fs / fmax), lmax = int(fs / fmin);
    double e0 = 0; for (double v : y) e0 += v * v;
    if (e0 <= 0) { *clarity = 0; return 0; }
    std::vector<double> r(size_t(lmax) + 2, 0.0);
    for (int l = lmin - 1; l <= lmax + 1 && size_t(l) < n; ++l) {
        double s = 0, ea = 0, eb = 0;
        for (size_t i = 0; i + size_t(l) < n; ++i) { s += y[i] * y[i + l]; ea += y[i] * y[i]; eb += y[i + l] * y[i + l]; }
        r[size_t(l)] = s / std::sqrt(ea * eb + 1e-300);
    }
    // first lag where r exceeds 0.8 * global max, then local max (avoids octave errors)
    double gmax = -1; for (int l = lmin; l <= lmax; ++l) gmax = std::max(gmax, r[size_t(l)]);
    int best = -1;
    for (int l = lmin; l <= lmax; ++l) {
        if (r[size_t(l)] >= 0.85 * gmax && r[size_t(l)] >= r[size_t(l - 1)] && r[size_t(l)] >= r[size_t(l + 1)]) { best = l; break; }
    }
    if (best < 0) { *clarity = 0; return 0; }
    const double a = r[size_t(best - 1)], b = r[size_t(best)], c = r[size_t(best + 1)];
    const double den = a - 2 * b + c, d = den != 0 ? 0.5 * (a - c) / den : 0;
    *clarity = b;
    return fs / (best + d);
}

int main(int argc, char** argv) {
    if (argc < 2) { std::fprintf(stderr, "usage: lipsim modes.txt k=v...\n"); return 2; }
    std::map<std::string, std::string> kv;
    for (int i = 2; i < argc; ++i) { std::string s = argv[i]; auto e = s.find('='); if (e != std::string::npos) kv[s.substr(0, e)] = s.substr(e + 1); }
    std::ifstream in(argv[1]);
    std::string head; in >> head;
    // Alternative input: first token "poles", then lines "Re(s) Im(s) Re(R) Im(R)" with dimensional residues
    // R (Pa s/m^3 per s, i.e. Zc*Ck of Freour et al. 2022 eq. 1), p = 2 Re sum q_k, q_k' = s_k q_k + R_k u.
    std::vector<cd> poles, residues;
    double Zc = 0;
    std::vector<Mode> modes;
    if (head == "poles") {
        double a, b2, c, d; while (in >> a >> b2 >> c >> d) { poles.emplace_back(a, b2); residues.emplace_back(c, d); }
    } else {
        Zc = std::atof(head.c_str());
        Mode m; while (in >> m.f >> m.Q >> m.A) modes.push_back(m);
    }

    const double fsOut = arg(kv, "fs", 48000), os = arg(kv, "os", 2), fs = fsOut * os, T = 1.0 / fs;
    const double dur = arg(kv, "dur", 0.8);
    const double fl0 = arg(kv, "fl", 233), fl1 = arg(kv, "fl_end", fl0);
    const double Ql = arg(kv, "Ql", 5), mu = arg(kv, "mu", 1.5), b = arg(kv, "b", 8e-3), H0v = arg(kv, "H", 1e-4);
    const double H1v = arg(kv, "H_end", H0v);
    const double pm0 = arg(kv, "pm", 5000), pm1 = arg(kv, "pm_end", pm0), rho = 1.2041;
    const double attack = arg(kv, "attack", 0.02);
    const double t0r = arg(kv, "ramp_start", 0.0);        // parameter ramps (fl, pm, H) start here
    const double rdur = arg(kv, "ramp_dur", -1);           // ramp length (s); <0 = until the end
    const double yinit = arg(kv, "yinit", -1);           // initial lip opening (tongue release); <0 = H
    const double kick = arg(kv, "kick", 0);              // initial modal excitation: Pa added to the mode nearest fl
    const double fscale = arg(kv, "fscale", 1.0);       // multiplies all mode frequencies (tuning)
    const int track = int(arg(kv, "track", 0));
    const double fmin = arg(kv, "fmin", 50), fmax = arg(kv, "fmax", 1500);

    // complex pole pairs, bilinear discretisation q[k+1] = a q[k] + g (u[k+1] + u[k]); p = sum 2 Re q
    for (auto& m : modes) {
        const double wn = 2 * M_PI * m.f, z = 1.0 / (2 * m.Q);
        const cd sig(-z * wn, wn * std::sqrt(std::max(0.0, 1 - z * z)));
        poles.push_back(sig);
        residues.push_back(Zc * m.A * (wn / m.Q) * sig / (sig - std::conj(sig)));  // residue of A wn/Q s/((s-sig)(s-sig*))
    }
    std::vector<cd> av, gv, q(poles.size(), 0.0);
    double Z0 = 0;
    for (size_t n = 0; n < poles.size(); ++n) {
        const cd sig(poles[n].real() * fscale, poles[n].imag() * fscale), R = residues[n] * fscale;
        const cd a = (1.0 + sig * T / 2.0) / (1.0 - sig * T / 2.0), g = R * (T / 2.0) / (1.0 - sig * T / 2.0);
        av.push_back(a); gv.push_back(g); Z0 += 2 * g.real();
    }
    double y = yinit >= 0 ? yinit : H0v, v = 0, u = 0, p = 0;
    if (kick != 0) {
        size_t best = 0; double bd = 1e300;
        for (size_t n = 0; n < poles.size(); ++n) { const double d = std::fabs(poles[n].imag() * fscale / (2 * M_PI) - fl0 * 1.03); if (d < bd) { bd = d; best = n; } }
        q[best] = cd(0, -kick / 2);  // p = 2 Re q: starts at 0 with a sine-phase oscillation of amplitude ~kick
    }
    const long N = long(dur * fs);
    std::vector<double> out; out.reserve(size_t(N / os) + 1);
    for (long k = 0; k < N; ++k) {
        const double t = double(k) * T, frac = t < t0r ? 0.0 : std::min(1.0, (t - t0r) / (rdur > 0 ? rdur : dur - t0r));
        const double env = t < attack ? 0.5 * (1 - std::cos(M_PI * t / attack)) : 1.0;
        const double pm = env * (pm0 + (pm1 - pm0) * frac);
        const double fl = fl0 * std::pow(fl1 / fl0, frac), wl = 2 * M_PI * fl;
        const double H = H0v + (H1v - H0v) * frac;
        // lip: semi-implicit Euler with the previous pressure
        const double acc = (pm - p) / mu - (wl / Ql) * v - wl * wl * (y - H);
        v += T * acc; y += T * v;
        const double h = std::max(y, 0.0);
        // implicit flow: p = P0 + Z0 u, u = c sign(D) sqrt(|pm - p|)
        double P0 = 0; for (size_t n = 0; n < q.size(); ++n) P0 += 2 * (av[n] * q[n] + gv[n] * u).real();
        const double c = b * h * std::sqrt(2.0 / rho), D = pm - P0;
        double unew;
        if (c <= 0) unew = 0;
        else if (D >= 0) { const double s = 0.5 * (-Z0 * c + std::sqrt(Z0 * Z0 * c * c + 4 * D)); unew = c * s; }
        else { const double s = 0.5 * (-Z0 * c + std::sqrt(Z0 * Z0 * c * c - 4 * D)); unew = -c * s; }
        for (size_t n = 0; n < q.size(); ++n) q[n] = av[n] * q[n] + gv[n] * (unew + u);
        u = unew; p = P0 + Z0 * u;
        if (!std::isfinite(p)) { std::printf("{\"error\":\"nonfinite\",\"t\":%.4f}\n", t); return 1; }
        if (k % long(os) == 0) out.push_back(p);
    }
    if (kv.count("wav")) {
        std::ofstream w(kv["wav"], std::ios::binary);
        double mx = 1e-9; for (double s : out) mx = std::max(mx, std::fabs(s));
        if (arg(kv, "raw", 0) != 0) mx = 1.0;   // raw=1: write pressure in Pa
        for (double s : out) { float f = float(s / mx); w.write(reinterpret_cast<char*>(&f), 4); }
    }
    auto summary = [&](size_t s0, size_t s1, double tsec) {
        std::vector<double> seg(out.begin() + long(s0), out.begin() + long(s1));
        double cl = 0, f0 = f0_acf(seg, fsOut, fmin, fmax, &cl);
        double mean = 0; for (double s : seg) mean += s; mean /= double(seg.size());
        double rms = 0; for (double s : seg) rms += (s - mean) * (s - mean); rms = std::sqrt(rms / double(seg.size()));
        const double fl = fl0 * std::pow(fl1 / fl0, tsec < t0r ? 0.0 : std::min(1.0, (tsec - t0r) / (rdur > 0 ? rdur : dur - t0r)));
        std::printf("{\"t\":%.3f,\"fl\":%.2f,\"f0\":%.2f,\"clarity\":%.3f,\"rms\":%.1f}\n", tsec, fl, f0, cl, rms);
    };
    if (track) {
        const size_t w = size_t(0.04 * fsOut), hop = size_t(0.025 * fsOut);
        for (size_t s = 0; s + w <= out.size(); s += hop) summary(s, s + w, double(s + w / 2) / fsOut);
    } else {
        const size_t s0 = out.size() * 2 / 3;
        summary(s0, out.size(), dur);
    }
    return 0;
}
