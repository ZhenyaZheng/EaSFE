#include "EaSFE/util/MyMath.h"
namespace EaSFE {

	double Gamma::gamma(double x) {
		if (x == rint(x) && x <= 0.0) {
			return 0.0 * 0.0;
		}
		else
		{
			double absX = abs(x);
			double ret;
			double prod;
			double t;
			if (absX <= 20.0) 
			{
				if (x >= 1.0) {
					prod = 1.0;
					for (t = x; t > 2.5; prod *= t) {
						--t;
					}
					ret = prod / (1.0 + invGamma1pm1(t - 1.0));
				}
				else {
					prod = x;
					for (t = x; t < -0.5; prod *= t) {
						++t;
					}
					ret = 1.0 / (prod * (1.0 + invGamma1pm1(t)));
				}
			}
			else 
			{
				prod = absX + 4.7421875 + 0.5;
				t = 2.5066282746310007 / x * pow(prod, absX + 0.5) * exp(-prod) * lanczos(absX);
				if (x > 0.0) {
					ret = t;
				}
				else {
					ret = -3.141592653589793 / (x * sin(3.141592653589793 * x) * t);
				}
			}
			return ret;
		}
	}
	double Gamma::invGamma1pm1(double x)
	{
		if (x < -0.5) {
			throw MyExcept("Gamma invGamma1pm1 Error: x < -0.5");
		}
		else if (x > 1.5) {
			throw MyExcept("Gamma invGamma1pm1 Error: x > 1.5");
		}
		else
		{
			double t = x <= 0.5 ? x : x - 0.5 - 0.5;
			double ret;
			double a;
			double b;
			double c;
			if (t < 0.0)
			{
				a = 6.116095104481416E-9 + t * 6.247308301164655E-9;
				b = 1.9575583661463974E-10;
				b = -6.077618957228252E-8 + t * b;
				b = 9.926418406727737E-7 + t * b;
				b = -6.4304548177935305E-6 + t * b;
				b = -8.514194324403149E-6 + t * b;
				b = 4.939449793824468E-4 + t * b;
				b = 0.026620534842894922 + t * b;
				b = 0.203610414066807 + t * b;
				b = 1.0 + t * b;
				c = -2.056338416977607E-7 + t * (a / b);
				c = 1.133027231981696E-6 + t * c;
				c = -1.2504934821426706E-6 + t * c;
				c = -2.013485478078824E-5 + t * c;
				c = 1.280502823881162E-4 + t * c;
				c = -2.1524167411495098E-4 + t * c;
				c = -0.0011651675918590652 + t * c;
				c = 0.0072189432466631 + t * c;
				c = -0.009621971527876973 + t * c;
				c = -0.04219773455554433 + t * c;
				c = 0.16653861138229148 + t * c;
				c = -0.04200263503409524 + t * c;
				c = -0.6558780715202539 + t * c;
				c = -0.42278433509846713 + t * c;
				if (x > 0.5) {
					ret = t * c / x;
				}
				else {
					ret = x * (c + 0.5 + 0.5);
				}
			}
			else
			{
				a = 4.343529937408594E-15;
				a = -1.2494415722763663E-13 + t * a;
				a = 1.5728330277104463E-12 + t * a;
				a = 4.686843322948848E-11 + t * a;
				a = 6.820161668496171E-10 + t * a;
				a = 6.8716741130671986E-9 + t * a;
				a = 6.116095104481416E-9 + t * a;
				b = 2.6923694661863613E-4;
				b = 0.004956830093825887 + t * b;
				b = 0.054642130860422966 + t * b;
				b = 0.3056961078365221 + t * b;
				b = 1.0 + t * b;
				c = -2.056338416977607E-7 + a / b * t;
				c = 1.133027231981696E-6 + t * c;
				c = -1.2504934821426706E-6 + t * c;
				c = -2.013485478078824E-5 + t * c;
				c = 1.280502823881162E-4 + t * c;
				c = -2.1524167411495098E-4 + t * c;
				c = -0.0011651675918590652 + t * c;
				c = 0.0072189432466631 + t * c;
				c = -0.009621971527876973 + t * c;
				c = -0.04219773455554433 + t * c;
				c = 0.16653861138229148 + t * c;
				c = -0.04200263503409524 + t * c;
				c = -0.6558780715202539 + t * c;
				c = 0.5772156649015329 + t * c;
				if (x > 0.5) {
					ret = t / x * (c - 0.5 - 0.5);
				}
				else {
					ret = x * c;
				}
			}

			return ret;
		}
	}
	double Gamma::logGamma1p(double x)
	{
		if (x < -0.5) 
		{
			throw MyExcept("Gamma logGamma1p Error: x < -0.5");
		}
		else if (x > 1.5) 
		{
			throw MyExcept("Gamma logGamma1p Error: x > 1.5");
		}
		else 
		{
			return -log1p(invGamma1pm1(x));
		}
	}
	double Gamma::lanczos(double x)
	{
		double sum = 0.0;
		std::vector<double> LANCZOS = {0.9999999999999971, 57.15623566586292, -59.59796035547549, 14.136097974741746, -0.4919138160976202, 3.399464998481189E-5, 4.652362892704858E-5, -9.837447530487956E-5, 1.580887032249125E-4, -2.1026444172410488E-4, 2.1743961811521265E-4, -1.643181065367639E-4, 8.441822398385275E-5, -2.6190838401581408E-5, 3.6899182659531625E-6};
		for (int i = LANCZOS.size() - 1; i > 0; --i) {
			sum += LANCZOS[i] / (x + (double)i);
		}

		return sum + LANCZOS[0];
	}
	double Gamma::logGamma(double x) {
		double ret;
		if (!isnan(x) && !(x <= 0.0)) {
			if (x < 0.5) {
				return logGamma1p(x) - log(x);
			}

			if (x <= 2.5) {
				return logGamma1p(x - 0.5 - 0.5);
			}

			if (x <= 8.0) {
				int n = (int)floor(x - 1.5);
				double prod = 1.0;

				for (int i = 1; i <= n; ++i) {
					prod *= x - (double)i;
				}

				return logGamma1p(x - (double)(n + 1)) + log(prod);
			}

			double sum = lanczos(x);
			double tmp = x + 4.7421875 + 0.5;
			ret = (x + 0.5) * log(tmp) - tmp + 0.5 * log(6.283185307179586) + log(sum / x);
		}
		else {
			ret = 0.0 * 0.0;
		}

		return ret;
	}



	double Beta::logGammaMinusLogGammaSum(double a, double b)
	{
		if (a < 0.0) {
			throw MyExcept("Beta logGammaMinusLogGammaSum Error: a < 0.0");
		}
		else if (b < 10.0) {
			throw MyExcept("Beta logGammaMinusLogGammaSum Error: b < 10.0");
		}
		else {
			double d;
			double w;
			if (a <= b) {
				d = b + (a - 0.5);
				w = deltaMinusDeltaSum(a, b);
			}
			else {
				d = a + (b - 0.5);
				w = deltaMinusDeltaSum(b, a);
			}

			double u = d * log1p(a / b);
			double v = a * (log(b) - 1.0);
			return u <= v ? w - u - v : w - v - u;
		}
	}
	double Beta::deltaMinusDeltaSum(double a, double b)
	{
		const std::vector<double> DELTA = { 0.08333333333333333, -2.777777777777778E-5, 7.936507936507937E-8, -5.952380952380953E-10, 8.417508417508329E-12, -1.917526917518546E-13, 6.410256405103255E-15, -2.955065141253382E-16, 1.7964371635940225E-17, -1.3922896466162779E-18, 1.338028550140209E-19, -1.542460098679661E-20, 1.9770199298095743E-21, -2.3406566479399704E-22, 1.713480149663986E-23 };
		if (!(a < 0.0) && !(a > b)) 
		{
			if (b < 10.0) 
			{
				throw MyExcept("Beta deltaMinusDeltaSum Error: b < 10.0");
			}
			else 
			{
				double h = a / b;
				double p = h / (1.0 + h);
				double q = 1.0 / (1.0 + h);
				double q2 = q * q;
				std::vector<double> s (DELTA.size());
				s[0] = 1.0;

				for (int i = 1; i < s.size(); ++i) {
					s[i] = 1.0 + q + q2 * s[i - 1];
				}

				double sqrtT = 10.0 / b;
				double t = sqrtT * sqrtT;
				double w = DELTA[DELTA.size() - 1] * s[s.size() - 1];

				for (int i = DELTA.size() - 2; i >= 0; --i) {
					w = t * w + DELTA[i] * s[i];
				}

				return w * p / b;
			}
		}
	else {
		throw MyExcept("Beta deltaMinusDeltaSum Error: OutOfRangeException");
	}
	}
	double Beta::sumDeltaMinusDeltaSum(double p, double q)
	{
		const std::vector<double> DELTA = { 0.08333333333333333, -2.777777777777778E-5, 7.936507936507937E-8, -5.952380952380953E-10, 8.417508417508329E-12, -1.917526917518546E-13, 6.410256405103255E-15, -2.955065141253382E-16, 1.7964371635940225E-17, -1.3922896466162779E-18, 1.338028550140209E-19, -1.542460098679661E-20, 1.9770199298095743E-21, -2.3406566479399704E-22, 1.713480149663986E-23 };
		if (p < 10.0) {
			throw MyExcept("Beta sumDeltaMinusDeltaSum Error: p < 10.0");
		}
		else if (q < 10.0) {
			throw MyExcept("Beta sumDeltaMinusDeltaSum Error: q < 10.0");
		}
		else {
			double a = std::min(p, q);
			double b = std::max(p, q);
			double sqrtT = 10.0 / a;
			double t = sqrtT * sqrtT;
			double z = DELTA[DELTA.size() - 1];

			for (int i = DELTA.size() - 2; i >= 0; --i) {
				z = t * z + DELTA[i];
			}

			return z / a + deltaMinusDeltaSum(a, b);
		}
	}
	double Beta::logGammaSum(double a, double b)
	{
		if (!(a < 1.0) && !(a > 2.0))
		{
			if (!(b < 1.0) && !(b > 2.0)) 
			{
				double x = a - 1.0 + (b - 1.0);
				if (x <= 0.5) {
					return Gamma::logGamma1p(1.0 + x);
				}
				else {
					return x <= 1.5 ? Gamma::logGamma1p(x) + log1p(x) : Gamma::logGamma1p(x - 1.0) + log(x * (1.0 + x));
				}
			}
			else {
				throw MyExcept("Beta logGammaSum Error: OutOfRangeException");
			}
		}
		else {
			throw MyExcept("Beta logGammaSum Error: OutOfRangeException");
		}
	}
	double Beta::logBeta(double p, double q)
	{
		if (!isnan(p) && !isnan(q) && !(p <= 0.0) && !(q <= 0.0)) {
			double a = std::min(p, q);
			double b = std::max(p, q);
			double prod1;
			double ared;
			double prod2;
			double bred;
			if (a >= 10.0) {
				prod1 = sumDeltaMinusDeltaSum(a, b);
				ared = a / b;
				prod2 = ared / (1.0 + ared);
				bred = -(a - 0.5) * log(prod2);
				double v = b * log1p(ared);
				return bred <= v ? -0.5 * log(b) + 0.9189385332046727 + prod1 - bred - v : -0.5 * log(b) + 0.9189385332046727 + prod1 - v - bred;
			}
			else if (a > 2.0) {
				if (b > 1000.0) {
					int n = (int)floor(a - 1.0);
					double prod = 1.0;
					double ared = a;

					for (int i = 0; i < n; ++i) {
						--ared;
						prod *= ared / (1.0 + ared / b);
					}

					return log(prod) - (double)n * log(b) + Gamma::logGamma(ared) + logGammaMinusLogGammaSum(ared, b);
				}
				else {
					prod1 = 1.0;

					for (ared = a; ared > 2.0; prod1 *= prod2 / (1.0 + prod2)) {
						--ared;
						prod2 = ared / b;
					}

					if (!(b < 10.0)) {
						return log(prod1) + Gamma::logGamma(ared) + logGammaMinusLogGammaSum(ared, b);
					}
					else {
						prod2 = 1.0;

						for (bred = b; bred > 2.0; prod2 *= bred / (ared + bred)) {
							--bred;
						}

						return log(prod1) + log(prod2) + Gamma::logGamma(ared) + (Gamma::logGamma(bred) - logGammaSum(ared, bred));
					}
				}
			}
			else if (!(a >= 1.0)) {
				return b >= 10.0 ? Gamma::logGamma(a) + logGammaMinusLogGammaSum(a, b) : log(Gamma::gamma(a) * Gamma::gamma(b) / Gamma::gamma(a + b));
			}
			else if (!(b > 2.0)) {
				return Gamma::logGamma(a) + Gamma::logGamma(b) - logGammaSum(a, b);
			}
			else if (!(b < 10.0)) {
				return Gamma::logGamma(a) + logGammaMinusLogGammaSum(a, b);
			}
			else {
				prod1 = 1.0;

				for (ared = b; ared > 2.0; prod1 *= ared / (a + ared)) {
					--ared;
				}

				return log(prod1) + Gamma::logGamma(a) + (Gamma::logGamma(ared) - logGammaSum(a, ared));
			}
		}
		else {
			return 0.0 * 0.0;
		}
	}



	long long Precision::doubleToRawBits(double x) {
		long long bits;
		memcpy(&bits, &x, sizeof bits);
		return bits;
	}
	bool Precision::equals(double x, double y, double eps) {
		return equals(x, y, 1) || abs(y - x) <= eps;
	}
	bool Precision::equals(double x, double y, int maxUlps) {
		long long xInt = doubleToRawBits(x);
		long long yInt = doubleToRawBits(y);
		if (xInt < 0) {
			xInt = -9223372036854775807LL - xInt;
		}

		if (yInt < 0) {
			yInt = -9223372036854775807LL - yInt;
		}

		bool isEqual = abs(xInt - yInt) <= (long long)maxUlps;
		return isEqual && !isnan(x) && !isnan(y);
	}


	ContinuedFraction::ContinuedFraction(double b, double a):m_b(b),m_a(a) { }
	double ContinuedFraction::getB(int n, double x)
	{
		double ret;
		double m;
		if (n % 2 == 0)
		{
			m = (double)n / 2.0;
			ret = m * (m_b - m) * x / ((m_a + 2.0 * m - 1.0) * (m_a + 2.0 * m));
		}
		else
		{
			m = ((double)n - 1.0) / 2.0;
			ret = -((m_a + m) * (m_a + m_b + m) * x) / ((m_a + 2.0 * m) * (m_a + 2.0 * m + 1.0));
		}

		return ret;
	}
	double ContinuedFraction::getA(int n, double x)
	{
		return 1.0;
	}
	double ContinuedFraction::evaluate(double x, double epsilon, int maxIterations)
	{
		double small_{ 1.0E-50 };
		double hPrev = getA(0, x);
		if (Precision::equals(hPrev, 0.0, 1.0E-50)) {
			hPrev = 1.0E-50;
		}

		int n = 1;
		double dPrev = 0.0;
		double cPrev = hPrev;
		double hN = hPrev;

		while (true)
		{
			if (n < maxIterations) {
				double a = getA(n, x);
				double b = getB(n, x);
				double dN = a + b * dPrev;
				if (Precision::equals(dN, 0.0, 1.0E-50)) {
					dN = 1.0E-50;
				}

				double cN = a + b / cPrev;
				if (Precision::equals(cN, 0.0, 1.0E-50)) {
					cN = 1.0E-50;
				}

				dN = 1.0 / dN;
				double deltaN = cN * dN;
				hN = hPrev * deltaN;
				if (!isfinite(hN)) {
					throw  MyExcept("ContinuedFraction evaluate : hN is finite");
				}

				if (isnan(hN)) {
					throw  MyExcept("ContinuedFraction evaluate : hN is nan");
				}

				if (!(abs(deltaN - 1.0) < epsilon)) {
					dPrev = dN;
					cPrev = cN;
					hPrev = hN;
					++n;
					continue;
				}
			}

			if (n >= maxIterations) {
				throw MyExcept("n >= maxIterations");
			}

			return hN;
		}
	}

}//EaSFE