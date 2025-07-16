#include <cassert>

#include "dtstruct.h"

const size_t k_max_load_factor = 8;
const size_t k_rehashing_work = 128;

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
  hm_trigger_rehashing(hmap);
  HNode **from = h_lookup(&hmap->newer, key, eq);
  if (!from) from = h_lookup(&hmap->older, key, eq);
  return from ? *from : Null;
}

void hm_insert(HMap *hmap, HNode *node) {
  if (!hmap->newer.tab) h_init(&hmap->newer, 4);

  h_insert(&hmap->newer, node);
  if (!hmap->older.tab) {
    size_t shreshold = (hmap->newer.mask + 1) * k_max_load_factor;
    if (hmap->newer.size >= shreshold) {
      hm_trigger_rehashing(hmap);
    }
  }
}

void hm_help_rehashing(HMap *hmap) {
  size_t nwork = 0;
  while (nwork < k_rehashing_work && hmap->older.size > 0) {
    HNode **from = &hmap->older.tab[hmap->migrate_pos];
    if (!*from) {
      hmap->migrate_pos++;
      continue;
    }
    h_insert(&hmap->newer, h_detach(&hmap->older, from));
    nwork++;
  }

  if (hmap->older.size == 0 hmap->older.tab) {
    free(hmap->older.tab);
    hmap->older = HTab{};
  }
}

bool entry_eq(HNode *lhs, HNode *rhs) {
  Entry *le = container_of(lhs, Entry, node);
  Entry *re = container_of(rhs, Entry, node);
  return le->key == re->key;
}

void do_get(std::vector<std::string> &cmd, Response &out) {
    // a dummy `Entry` just for the lookup
    Entry key;
    key.key.swap(cmd[1]);
    key.node.hcode = str_hash((uint8_t *)key.key.data(), key.key.size());
    // hashtable lookup
    HNode *node = hm_lookup(&g_data.db, &key.node, &entry_eq);
    if (!node) {
        out.status = RES_NX;
        return;
    }
    // copy the value
    const std::string &val = container_of(node, Entry, node)->val;
    assert(val.size() <= k_max_msg);
    out.data.assign(val.begin(), val.end());
}

HNode *hm_delete(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *)) {
  hm_trigger_rehashing(hmap);
  if (HNode **from = h_lookup(&hmap->newer, key, eq)) return h_detach(&hmap->newer, from);
  if (HNode **from = h_lookup(&hmap->older, key, eq)) return h_detach(&hmap->older, from);
  return NULL;
}

} //namespace Hash
