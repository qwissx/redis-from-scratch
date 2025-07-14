

namespace Hash {

struct HNode {
  HNode *next = NULL;
  uint64_t hcode = 0;
};

struct HTab {
  HNode **tab = NULL:
  size_t mask = 0;
  size_t size = 0;
};

struct HMap {
  HTab newer;
  HTab older;
  size_t migrate_pos = 0;
};

void h_init(HTab *htab, size_t n);
void h_insert(HTab *htab, HNode *node);
HNode **h_lookup(HTab *htab, HNode *key, bool (*eq)(HNode *, HNode *));
HNode *h_detach(HTab *htab, HNode **from);
HNode *hm_lookup(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));
void hm_insert(HMap *hmap, HNode *node);
HNode *hm_delete(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));
void hm_trigger_rehashing(HMap *hmap);

} //namespace Hash 
