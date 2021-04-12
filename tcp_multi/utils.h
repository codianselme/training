#ifndef __UTILS_H
#define __UTILS_H

int resolve_address(struct sockaddr *sa, socklen_t *salen, const char *host, 
  const char *port, int family, int type, int proto);
#endif /* __UTILS_H */
