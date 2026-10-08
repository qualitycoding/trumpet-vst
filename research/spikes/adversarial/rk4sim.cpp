// Independent reference: classical RK4 on the continuous-time system (no algebraic loop: p is a state function).
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <string>
#include <vector>
using cd = std::complex<double>;
int main(int argc, char** argv) {
    std::map<std::string, double> kv = {{"fl",382.18},{"Ql",3},{"mu",2},{"b",8e-3},{"H",1e-4},{"pm",3000},{"dur",0.8},
        {"attack",0.02},{"yinit",-1},{"fsint",2e6},{"fsout",48000},{"rho",1.2041},{"fscale",1}};
    for (int i = 3; i < argc; ++i) { std::string s = argv[i]; auto e = s.find('='); kv[s.substr(0,e)] = std::atof(s.c_str()+e+1); }
    std::ifstream in(argv[1]); std::string head; in >> head;
    std::vector<cd> S, R; double a,b2,c,d; while (in >> a >> b2 >> c >> d) { S.emplace_back(a*kv["fscale"], b2*kv["fscale"]); R.emplace_back(c*kv["fscale"], d*kv["fscale"]); }
    const size_t N = S.size(); const double T = 1.0/kv["fsint"];
    const double wl = 2*M_PI*kv["fl"], Ql=kv["Ql"], mu=kv["mu"], bw=kv["b"], H=kv["H"], rho=kv["rho"], att=kv["attack"], pm0=kv["pm"];
    struct St { double y, v; std::vector<cd> q; };
    auto pm_at = [&](double t){ return pm0 * (t < att ? 0.5*(1-std::cos(M_PI*t/att)) : 1.0); };
    auto deriv = [&](const St& x, double t, St& dx) {
        double p = 0; for (auto& q : x.q) p += 2*q.real();
        const double pm = pm_at(t), D = pm - p;
        const double u = bw*std::max(x.y,0.0)*(D>=0?1:-1)*std::sqrt(2*std::fabs(D)/rho);
        dx.y = x.v; dx.v = D/mu - (wl/Ql)*x.v - wl*wl*(x.y-H);
        dx.q.resize(N); for (size_t k=0;k<N;++k) dx.q[k] = S[k]*x.q[k] + R[k]*u;
    };
    St x{kv["yinit"]>=0?kv["yinit"]:H, 0, std::vector<cd>(N,0.0)};
    St k1,k2,k3,k4,tmp; tmp.q.resize(N);
    auto axpy=[&](const St& base, const St& k, double h, St& out){ out.y=base.y+h*k.y; out.v=base.v+h*k.v; out.q.resize(N); for(size_t i=0;i<N;++i) out.q[i]=base.q[i]+h*k.q[i]; };
    const long steps = long(kv["dur"]*kv["fsint"]); const long dec = long(kv["fsint"]/kv["fsout"]);
    std::vector<float> out;
    for (long n=0;n<steps;++n){
        double t=n*T;
        deriv(x,t,k1); axpy(x,k1,T/2,tmp); deriv(tmp,t+T/2,k2); axpy(x,k2,T/2,tmp); deriv(tmp,t+T/2,k3); axpy(x,k3,T,tmp); deriv(tmp,t+T,k4);
        x.y += T/6*(k1.y+2*k2.y+2*k3.y+k4.y); x.v += T/6*(k1.v+2*k2.v+2*k3.v+k4.v);
        for(size_t i=0;i<N;++i) x.q[i]+=T/6*(k1.q[i]+2.0*k2.q[i]+2.0*k3.q[i]+k4.q[i]);
        if (n % dec == 0) { double p=0; for(auto&q:x.q) p+=2*q.real(); out.push_back(float(p)); }
    }
    std::ofstream w(argv[2], std::ios::binary); w.write((char*)out.data(), out.size()*4);
}
