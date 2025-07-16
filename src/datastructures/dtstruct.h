#include <cstring>

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

struct {
  HMap db;
} g_data;

struct Entry {
  struct HNode node;
  std::string key;
  std::string val;
};


void do_get(std::vector<std::string> &cmd, Response &out);
void h_init(HTab *htab, size_t n);
void h_insert(HTab *htab, HNode *node);
HNode **h_lookup(HTab *htab, HNode *key, bool (*eq)(HNode *, HNode *));
HNode *h_detach(HTab *htab, HNode **from);
HNode *hm_lookup(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));
void hm_insert(HMap *hmap, HNode *node);
HNode *hm_delete(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));
void hm_help_rehashing(HMap *hmap);
bool entry_eq(HNode *lhs, HNode *rhs);
void do_get(std::vector<std::string> &cmd, Response &out);
void hm_trigger_rehashing(HMap *hmap);

} //namespace Hash 
