#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t ans=0;
  while(*s != '\0'){
    ans++;
    s++;
  }
  return ans;
}

char *strcpy(char* dst,const char* src) {
  size_t i;
  for(i=0;src[i]!='\0';i++){
    dst[i]=src[i];
  }
  dst[i+1]='\0';
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  size_t i;
  for(i=0;i<n&&src[i]!='\0';i++) 
    dst[i]=src[i];
  for(;i<n;i++)
    dst[i]='\0';
  return dst;
}

char* strcat(char* dst, const char* src) {
  size_t i;
  size_t dst_l=strlen(dst);
  for(i=0;src[i]!='\0';i++)
    dst[dst_l+i]=src[i];
  dst[dst_l+i]='\0';
  return dst;
}

int strcmp(const char* s1, const char* s2) {
  size_t i;
  for(i=0;;i++){
    if(s1[i]<s2[i]) return -1;
    if(s1[i]>s2[i]) return 1;
    if(s1[i]=='\0' && s2[i]=='\0') return 0;
  }
}

int strncmp(const char* s1, const char* s2, size_t n) {
  size_t i;
  for(i=0;i<n;i++){
    if(s1[i]<s2[i]) return -1;
    if(s1[i]>s2[i]) return 1;
    if(s1[i]=='\0' && s2[i]=='\0') return 0;
  }
  return 0;
}

void* memset(void* v,int c,size_t n) {
  size_t i;
  char *tv=(char*)v;
  for(i=0;i<n;i++)
    *(tv+i)=c;
  return v;
}

void* memcpy(void* out, const void* in, size_t n) {
  char *tout=(char*)out;
  char *tin=(char*)in;
  while(n--){
    *tout=*tin;
    tout++;tin++;
  }
  return out;
}

int memcmp(const void* s1, const void* s2, size_t n){
  size_t i;
  unsigned char* ts1=(unsigned char*)s1;
  unsigned char* ts2=(unsigned char*)s2;
  for(i=0;i<n;i++){
    if(ts1[i]<ts2[i]) return -1;
    if(ts1[i]>ts2[i]) return 1;
    if(ts1[i]=='\0' && ts2[i]=='\0') return 0;
  }
  return 0;
}

#endif
