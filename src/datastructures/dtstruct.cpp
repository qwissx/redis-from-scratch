#include <cassert>

#include "dtstruct.h"

namespace Hash {
void h_init(HTab *htab, size_t n) {
  assert(n > 0 && ((n - 1) & n) == 0);
  htab->tab = (HNode **)calloc(n, sizeof(HNode *));
  htab->mask = n - 1;
  htab->size = 0;
}

void h_insert(HTab *htab, HNode *node) {
  size_t pos = node->hcode & htab->mask;
  HNode *next = htab->tab[pos];
  node->next = next;
  htab->tab[pos] = node;
  htab->size++;
}

HNode **h_lookup(HTab *htab, HNode *key, bool (*eq)(HNode *, HNode *)) {
  if (!htab->tab) return NULL;

  size_t pos = key->hcode & htab->mask;
  HNode **from = &htab->tab[pos];

  for (HNode *cur; (cur = *from) != NULL; from = &cur->next) {
    if (cur->hcode == key->hcode && eq(cur, key)) {
      return from;
    }
  }
  return NULL;
}

HNode *h_detach(HTab *htab, HNode **from) {
  HNode *node = *from;
  *from = node->next;
  htab->size--;
  return node;
}

void hm_trigger_rehashing(HMap *hmap) {
  hmap->older = hmap->newer;
  h_init(&hmap->newer, (hmap->newer.mask + 1) * 2);
  hmap->migrate_pos = 0;
}

HNode *hm_lookup(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *)) {
  HNode **from = h_lookup(&hmap->newer, key, eq);
  if (!from) from = h_lookup(&hmap->older, key, eq);
  return from ? *from : Null;
}

void hm_insert(HMap *hmap, HNode *node) {

}

HNode *hm_delete(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *)) {
  if (HNode **from = h_lookup(&hmap->newer, key, eq)) return h_detach(&hmap->newer, from);
  if (HNode **from = h_lookup(&hmap->older, key, eq)) return h_detach(&hmap->older, from);
  return NULL;
}

} //namespace Hash
