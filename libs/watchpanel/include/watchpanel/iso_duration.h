#ifndef WATCHPANEL_ISO_DURATION_H_
#define WATCHPANEL_ISO_DURATION_H_

namespace watchpanel {

// Parses a (deliberately limited) subset of ISO-8601 durations into whole
// seconds: P[n]D optionally followed by T[n]H[n]M[n]S, e.g. "PT15M" (15
// minutes), "P1D" (1 day), "PT24H" (24 hours), "P1DT2H30M". Year/month
// components are intentionally unsupported -- they aren't a fixed number
// of seconds, which makes them meaningless for a cache TTL. Returns false
// (leaving *seconds untouched) on anything else, including a null/empty
// string.
bool ParseIsoDuration(const char *iso, long *seconds);

}  // namespace watchpanel

#endif  // WATCHPANEL_ISO_DURATION_H_
