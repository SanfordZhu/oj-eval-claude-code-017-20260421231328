// Train Ticket Management System - ACMOJ 1170
// In-memory implementation using custom containers (no STL containers except std::string)
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <string>

// ------------- Custom containers -------------
template <typename T>
class Vector {
    T* data_;
    int size_, cap_;
    void grow(int need) {
        int nc = cap_ ? cap_ : 4;
        while (nc < need) nc *= 2;
        T* nd = (T*)operator new(sizeof(T) * nc);
        for (int i = 0; i < size_; ++i) { new (nd + i) T(static_cast<T&&>(data_[i])); data_[i].~T(); }
        operator delete(data_);
        data_ = nd; cap_ = nc;
    }
public:
    Vector() : data_(nullptr), size_(0), cap_(0) {}
    Vector(const Vector& o) : data_(nullptr), size_(0), cap_(0) {
        if (o.size_) { grow(o.size_); for (int i=0;i<o.size_;++i) new(data_+i)T(o.data_[i]); size_=o.size_; }
    }
    Vector& operator=(const Vector& o) {
        if (this==&o) return *this;
        clear();
        if (o.size_) { grow(o.size_); for (int i=0;i<o.size_;++i) new(data_+i)T(o.data_[i]); size_=o.size_; }
        return *this;
    }
    ~Vector() { clear(); operator delete(data_); }
    void clear() { for (int i=0;i<size_;++i) data_[i].~T(); size_=0; }
    int size() const { return size_; }
    bool empty() const { return size_==0; }
    void push_back(const T& v) { if (size_+1>cap_) grow(size_+1); new(data_+size_)T(v); ++size_; }
    void pop_back() { --size_; data_[size_].~T(); }
    T& operator[](int i) { return data_[i]; }
    const T& operator[](int i) const { return data_[i]; }
    T& back() { return data_[size_-1]; }
    void erase(int idx) {
        for (int i=idx;i+1<size_;++i) data_[i]=data_[i+1];
        --size_; data_[size_].~T();
    }
};

// Simple fixed-size string for keys
struct FixStr {
    char s[32];
    FixStr(){ s[0]=0; }
    FixStr(const char* c){ strncpy(s,c,31); s[31]=0; }
    FixStr(const std::string& c){ strncpy(s,c.c_str(),31); s[31]=0; }
    bool operator<(const FixStr&o) const { return strcmp(s,o.s)<0; }
    bool operator==(const FixStr&o) const { return strcmp(s,o.s)==0; }
    bool operator!=(const FixStr&o) const { return strcmp(s,o.s)!=0; }
};

// AVL-tree-based map
template <typename K, typename V>
class Map {
    struct Node { K k; V v; Node *l,*r; int h; Node(const K&k_,const V&v_):k(k_),v(v_),l(nullptr),r(nullptr),h(1){} };
    Node* root_;
    int sz_;
    static int H(Node* n){return n?n->h:0;}
    static void upd(Node* n){n->h=1+(H(n->l)>H(n->r)?H(n->l):H(n->r));}
    static Node* rR(Node* y){Node* x=y->l; y->l=x->r; x->r=y; upd(y); upd(x); return x;}
    static Node* rL(Node* x){Node* y=x->r; x->r=y->l; y->l=x; upd(x); upd(y); return y;}
    static Node* bal(Node* n){ upd(n); int b=H(n->l)-H(n->r);
        if (b>1){ if (H(n->l->l)<H(n->l->r)) n->l=rL(n->l); return rR(n);}
        if (b<-1){ if (H(n->r->r)<H(n->r->l)) n->r=rR(n->r); return rL(n);}
        return n;
    }
    Node* ins(Node* n, const K&k, const V&v, bool& inserted){
        if (!n){ inserted=true; ++sz_; return new Node(k,v); }
        if (k<n->k) n->l=ins(n->l,k,v,inserted);
        else if (n->k<k) n->r=ins(n->r,k,v,inserted);
        else { n->v=v; inserted=false; return n; }
        return bal(n);
    }
    static Node* minN(Node* n){ while(n->l) n=n->l; return n; }
    Node* del(Node* n, const K&k, bool& removed){
        if (!n) return nullptr;
        if (k<n->k) n->l=del(n->l,k,removed);
        else if (n->k<k) n->r=del(n->r,k,removed);
        else {
            removed=true; --sz_;
            if (!n->l||!n->r){ Node* t=n->l?n->l:n->r; delete n; return t; }
            Node* m=minN(n->r); n->k=m->k; n->v=m->v;
            bool r2=false; n->r=del(n->r,m->k,r2);
        }
        return bal(n);
    }
    void clearN(Node* n){ if(!n) return; clearN(n->l); clearN(n->r); delete n; }
    void inorder(Node* n, Vector<Node*>& out) const { if(!n) return; inorder(n->l,out); out.push_back(n); inorder(n->r,out); }
public:
    Map():root_(nullptr),sz_(0){}
    ~Map(){ clearN(root_); }
    int size() const { return sz_; }
    V* find(const K& k){
        Node* n=root_;
        while(n){ if (k<n->k) n=n->l; else if (n->k<k) n=n->r; else return &n->v; }
        return nullptr;
    }
    bool insert(const K&k, const V&v){ bool ins_=false; root_=ins(root_,k,v,ins_); return ins_; }
    bool erase(const K&k){ bool r=false; root_=del(root_,k,r); return r; }
    void collect(Vector<Node*>& out) const { inorder(root_, out); }
    void clear(){ clearN(root_); root_=nullptr; sz_=0; }
};

// ------------- Parsing utilities -------------
static int parseInt(const char* s){ int v=0,sign=1; if(*s=='-'){sign=-1;++s;} while(*s>='0'&&*s<='9'){v=v*10+(*s-'0');++s;} return v*sign; }

// Time in minutes since 2021-06-01 00:00
static const int daysBefore[] = {0,0,0,0,0,0,0,30,61,92}; // month 6 -> 0, 7->30, 8->61
static int mmddToDay(const char* s){ // "mm-dd"
    int m=(s[0]-'0')*10+(s[1]-'0'); int d=(s[3]-'0')*10+(s[4]-'0');
    return daysBefore[m] + d - 1;
}
static int hhmmToMin(const char* s){ int h=(s[0]-'0')*10+(s[1]-'0'); int mi=(s[3]-'0')*10+(s[4]-'0'); return h*60+mi; }
static void dayToMMDD(int day, char* out){ // day offset since 06-01
    int m=6,d=day+1;
    int md[]={0,0,0,0,0,0,30,31,31};
    while (d>md[m]){ d-=md[m]; ++m; }
    out[0]='0'+m/10; out[1]='0'+m%10; out[2]='-'; out[3]='0'+d/10; out[4]='0'+d%10; out[5]=0;
}
static void absMinToStr(long long mins, char* out){ // minutes since 2021-06-01 00:00
    int day = (int)(mins/1440);
    int inday = (int)(mins%1440);
    int h=inday/60, mi=inday%60;
    char d[8]; dayToMMDD(day, d);
    out[0]=d[0]; out[1]=d[1]; out[2]=d[2]; out[3]=d[3]; out[4]=d[4]; out[5]=' ';
    out[6]='0'+h/10; out[7]='0'+h%10; out[8]=':'; out[9]='0'+mi/10; out[10]='0'+mi%10; out[11]=0;
}

// Split a string by delimiter
static int splitBy(char* s, char delim, char** out, int maxN){
    int n=0; out[n++]=s;
    for (char* p=s; *p; ++p){ if (*p==delim){ *p=0; if(n<maxN) out[n++]=p+1; } }
    return n;
}

// ------------- Domain objects -------------
struct User {
    char username[24];
    char password[36];
    char name[24]; // up to 5 Chinese chars = 15 bytes UTF-8
    char mail[36];
    int privilege;
};

struct Train {
    char trainID[24];
    int stationNum;
    int seatNum;
    char stations[100][16]; // 10 chinese chars max = 30 bytes, keep 16? adjust
    int prices[100];        // prices[i] = price station_i -> station_{i+1}; we store cumulative
    int startTime;          // minutes in day
    int travel[100];        // travel[i] = minutes between i and i+1
    int stopover[100];      // stopover[i] = stopover at i (1..n-2), stopover[0]=stopover[n-1]=0
    int saleStart, saleEnd; // day offsets
    char type;
    bool released;
    // prefix price and prefix time (arrive at station i from start in minutes)
    int prefixPrice[100];
    int arriveMin[100];     // absolute min-of-day from startTime=0 baseline (relative), arrival at station i
    int leaveMin[100];      // leaving minute (relative)
    // seats[day][segment] - dynamic: allocate when released
    int* seats; // size = (saleEnd-saleStart+1) * (stationNum-1)
    int numDays() const { return saleEnd-saleStart+1; }
    int segmentsCount() const { return stationNum-1; }
};

// Station -> list of (trainIdx, station index)
struct StationEntry { int trainIdx; int stationIdx; };

// Order
struct Order {
    int status; // 0 success, 1 pending, 2 refunded
    char trainID[24];
    char fromSt[16];
    char toSt[16];
    int fromIdx, toIdx;
    int trainIdx;
    int day;  // day offset for train start (so seats index)
    int num;
    int price; // unit price (cumulative)
    long long leaveAbs, arriveAbs;
    int orderSeq; // global ordering
};

// Pending entry per train/day
struct Pending {
    int userIdx;
    int orderIdx; // index into user's orders
    int fromIdx, toIdx;
    int num;
};

// ------------- Global state -------------
static Map<FixStr, int> userMap;   // username -> index
static Vector<User> users;
static Vector<Vector<int>> userOrders; // per-user order list (indices into global orders? no -> per-user orders vector)
// per-user orders stored directly
static Vector<Vector<Order>> allOrders; // [userIdx] -> orders (newest last)
static Map<FixStr, bool> loggedIn; // username -> true

static Map<FixStr, int> trainMap; // trainID -> train idx
static Vector<Train*> trains;     // pointer to avoid heavy copying

static Map<FixStr, Vector<StationEntry>> stationIndex;

// Per train, per day: list of pending orders
// use dynamic: trainIdx -> Vector<Vector<Pending>>
static Vector<Vector<Vector<Pending>>> pendingLists;

static int globalOrderSeq = 0;

// ------------- Helpers -------------
static void computePrefix(Train& t){
    t.prefixPrice[0]=0;
    for (int i=1;i<t.stationNum;++i) t.prefixPrice[i]=t.prefixPrice[i-1]+t.prices[i-1];
    // Times relative to the day of departure from origin station
    // station 0: leave = startTime, arrive = undefined
    t.leaveMin[0]=t.startTime;
    t.arriveMin[0]=-1;
    int cur = t.startTime;
    for (int i=1;i<t.stationNum;++i){
        cur += t.travel[i-1]; // arrive at station i
        t.arriveMin[i]=cur;
        if (i<t.stationNum-1){
            cur += t.stopover[i-1]; // stopover at station i (index in stopover arr is i-1)
            t.leaveMin[i]=cur;
        } else {
            t.leaveMin[i]=-1;
        }
    }
}

// Process pending queue when seats freed on trainIdx, day
static void tryFulfillPending(int trainIdx, int day);

// Find parameter from tokens
// tokens: array of strings with -x patterns
struct Cmd {
    char name[32];
    const char* args[26]; // a..z
    void init(){ name[0]=0; for (int i=0;i<26;++i) args[i]=nullptr; }
};

static bool parseCmd(char* line, Cmd& c){
    c.init();
    // skip whitespace
    char* p=line; while (*p==' '||*p=='\t') ++p;
    if (!*p||*p=='\n') return false;
    // get name
    char* q=c.name;
    while (*p && *p!=' '&&*p!='\t'&&*p!='\n'){ *q++=*p++; }
    *q=0;
    // parse -k v pairs
    while (*p){
        while (*p==' '||*p=='\t'||*p=='\n') ++p;
        if (!*p) break;
        if (*p!='-') break;
        ++p; // after -
        char key=*p++;
        while (*p==' '||*p=='\t') ++p;
        const char* argStart=p;
        while (*p && *p!=' '&&*p!='\t'&&*p!='\n') ++p;
        if (*p){ *p=0; ++p; }
        if (key>='a'&&key<='z') c.args[key-'a']=argStart;
    }
    return true;
}

// Convenience
static const char* A(const Cmd& c, char k){ return c.args[k-'a']; }

// ------------- Command implementations -------------

static int cmd_add_user(const Cmd& c){
    const char* u = A(c,'u');
    if (!u) return -1;
    FixStr key(u);
    if (userMap.find(key)) return -1;
    User us; memset(&us,0,sizeof(us));
    strncpy(us.username, u, 23);
    if (A(c,'p')) strncpy(us.password, A(c,'p'), 35);
    if (A(c,'n')) strncpy(us.name, A(c,'n'), 23);
    if (A(c,'m')) strncpy(us.mail, A(c,'m'), 35);
    if (users.size()==0){
        us.privilege = 10;
    } else {
        const char* cu = A(c,'c'); if (!cu) return -1;
        FixStr ck(cu);
        if (!loggedIn.find(ck)) return -1;
        int* curIdx = userMap.find(ck);
        if (!curIdx) return -1;
        int g = A(c,'g')?parseInt(A(c,'g')):-1;
        if (g<0||g>=users[*curIdx].privilege) return -1;
        us.privilege = g;
    }
    int idx = users.size();
    users.push_back(us);
    allOrders.push_back(Vector<Order>());
    userMap.insert(key, idx);
    return 0;
}

static int cmd_login(const Cmd& c){
    const char* u=A(c,'u'); const char* p=A(c,'p');
    if (!u||!p) return -1;
    FixStr k(u);
    int* idx = userMap.find(k);
    if (!idx) return -1;
    if (strcmp(users[*idx].password, p)!=0) return -1;
    if (loggedIn.find(k)) return -1;
    loggedIn.insert(k, true);
    return 0;
}

static int cmd_logout(const Cmd& c){
    const char* u=A(c,'u'); if (!u) return -1;
    FixStr k(u);
    if (!loggedIn.find(k)) return -1;
    loggedIn.erase(k);
    return 0;
}

static int cmd_query_profile(const Cmd& c, char* outbuf){
    const char* cu=A(c,'c'); const char* u=A(c,'u');
    if (!cu||!u) return -1;
    FixStr ck(cu), uk(u);
    if (!loggedIn.find(ck)) return -1;
    int* ci = userMap.find(ck); int* ui = userMap.find(uk);
    if (!ci||!ui) return -1;
    if (*ci!=*ui && users[*ci].privilege<=users[*ui].privilege) return -1;
    sprintf(outbuf, "%s %s %s %d", users[*ui].username, users[*ui].name, users[*ui].mail, users[*ui].privilege);
    return 0;
}

static int cmd_modify_profile(const Cmd& c, char* outbuf){
    const char* cu=A(c,'c'); const char* u=A(c,'u');
    if (!cu||!u) return -1;
    FixStr ck(cu), uk(u);
    if (!loggedIn.find(ck)) return -1;
    int* ci = userMap.find(ck); int* ui = userMap.find(uk);
    if (!ci||!ui) return -1;
    if (*ci!=*ui && users[*ci].privilege<=users[*ui].privilege) return -1;
    int newg = A(c,'g')?parseInt(A(c,'g')):-1;
    if (A(c,'g') && newg>=users[*ci].privilege) return -1;
    if (A(c,'p')) strncpy(users[*ui].password, A(c,'p'), 35);
    if (A(c,'n')) strncpy(users[*ui].name, A(c,'n'), 23);
    if (A(c,'m')) strncpy(users[*ui].mail, A(c,'m'), 35);
    if (A(c,'g')) users[*ui].privilege = newg;
    sprintf(outbuf, "%s %s %s %d", users[*ui].username, users[*ui].name, users[*ui].mail, users[*ui].privilege);
    return 0;
}

static int cmd_add_train(const Cmd& c){
    const char* i_=A(c,'i');
    if (!i_) return -1;
    FixStr key(i_);
    if (trainMap.find(key)) return -1;
    Train* t = new Train();
    memset(t,0,sizeof(Train));
    strncpy(t->trainID, i_, 23);
    t->stationNum = parseInt(A(c,'n'));
    t->seatNum = parseInt(A(c,'m'));
    t->type = A(c,'y')?A(c,'y')[0]:' ';
    t->released = false;
    t->seats = nullptr;
    // parse stations
    {
        char buf[4096]; strncpy(buf, A(c,'s'), 4095); buf[4095]=0;
        char* parts[101]; int n = splitBy(buf, '|', parts, 101);
        for (int k=0;k<n && k<t->stationNum;++k){ strncpy(t->stations[k], parts[k], 15); }
    }
    {
        char buf[4096]; strncpy(buf, A(c,'p'), 4095); buf[4095]=0;
        char* parts[101]; int n = splitBy(buf, '|', parts, 101);
        for (int k=0;k<n;++k) t->prices[k]=parseInt(parts[k]);
    }
    {
        const char* x = A(c,'x'); t->startTime = hhmmToMin(x);
    }
    {
        char buf[4096]; strncpy(buf, A(c,'t'), 4095); buf[4095]=0;
        char* parts[101]; int n = splitBy(buf, '|', parts, 101);
        for (int k=0;k<n;++k) t->travel[k]=parseInt(parts[k]);
    }
    {
        const char* o = A(c,'o');
        if (t->stationNum>2){
            char buf[4096]; strncpy(buf, o, 4095); buf[4095]=0;
            char* parts[101]; int n = splitBy(buf, '|', parts, 101);
            for (int k=0;k<n;++k) t->stopover[k]=parseInt(parts[k]);
        }
    }
    {
        char buf[64]; strncpy(buf, A(c,'d'), 63); buf[63]=0;
        char* parts[2]; splitBy(buf, '|', parts, 2);
        t->saleStart = mmddToDay(parts[0]);
        t->saleEnd = mmddToDay(parts[1]);
    }
    computePrefix(*t);
    int idx = trains.size();
    trains.push_back(t);
    trainMap.insert(key, idx);
    pendingLists.push_back(Vector<Vector<Pending>>());
    return 0;
}

static int cmd_release_train(const Cmd& c){
    const char* i_=A(c,'i'); if (!i_) return -1;
    FixStr k(i_);
    int* idx = trainMap.find(k); if (!idx) return -1;
    Train* t = trains[*idx];
    if (t->released) return -1;
    t->released = true;
    int nd = t->numDays(); int seg = t->segmentsCount();
    t->seats = new int[nd*seg];
    for (int i=0;i<nd*seg;++i) t->seats[i]=t->seatNum;
    // index stations
    for (int s=0;s<t->stationNum;++s){
        FixStr sk(t->stations[s]);
        Vector<StationEntry>* v = stationIndex.find(sk);
        if (!v){ stationIndex.insert(sk, Vector<StationEntry>()); v=stationIndex.find(sk); }
        StationEntry e; e.trainIdx=*idx; e.stationIdx=s;
        v->push_back(e);
    }
    // init pending per day
    Vector<Vector<Pending>> pd;
    for (int i=0;i<nd;++i) pd.push_back(Vector<Pending>());
    pendingLists[*idx] = pd;
    return 0;
}

static int cmd_delete_train(const Cmd& c){
    const char* i_=A(c,'i'); if (!i_) return -1;
    FixStr k(i_);
    int* idx = trainMap.find(k); if (!idx) return -1;
    if (trains[*idx]->released) return -1;
    delete trains[*idx]; trains[*idx]=nullptr;
    trainMap.erase(k);
    return 0;
}

// For before-release query_train: show seats=seatNum, cumPrice, etc.
static int cmd_query_train(const Cmd& c, char* outbuf){
    const char* i_=A(c,'i'); const char* d=A(c,'d');
    if (!i_||!d) return -1;
    FixStr k(i_);
    int* idx = trainMap.find(k); if (!idx || !trains[*idx]) return -1;
    Train* t = trains[*idx];
    int day = mmddToDay(d);
    if (day<t->saleStart||day>t->saleEnd) return -1;
    char* w=outbuf;
    w += sprintf(w, "%s %c\n", t->trainID, t->type);
    long long dayAbs = (long long)day*1440;
    for (int s=0;s<t->stationNum;++s){
        char arr[16], lve[16];
        if (s==0) strcpy(arr,"xx-xx xx:xx"); else absMinToStr(dayAbs+t->arriveMin[s], arr);
        if (s==t->stationNum-1) strcpy(lve,"xx-xx xx:xx"); else absMinToStr(dayAbs+t->leaveMin[s], lve);
        int seat; char seatBuf[16];
        if (s==t->stationNum-1){ strcpy(seatBuf,"x"); }
        else {
            if (!t->released) seat = t->seatNum;
            else seat = t->seats[(day - t->saleStart)*t->segmentsCount() + s];
            sprintf(seatBuf,"%d",seat);
        }
        w += sprintf(w, "%s %s -> %s %d %s", t->stations[s], arr, lve, t->prefixPrice[s], seatBuf);
        if (s<t->stationNum-1) *w++='\n';
    }
    *w=0;
    return 0;
}

// Compute seat availability for train on given startDay in [fromIdx, toIdx)
static int minSeats(Train* t, int startDay, int fromIdx, int toIdx){
    int seg = t->segmentsCount();
    int base = (startDay - t->saleStart)*seg;
    int mn = 0x7fffffff;
    for (int i=fromIdx;i<toIdx;++i){
        int s = t->seats[base+i];
        if (s<mn) mn=s;
    }
    return mn;
}

static void applySeats(Train* t, int startDay, int fromIdx, int toIdx, int delta){
    int seg = t->segmentsCount();
    int base = (startDay - t->saleStart)*seg;
    for (int i=fromIdx;i<toIdx;++i) t->seats[base+i] += delta;
}

static int cmd_query_ticket(const Cmd& c, char* outbuf){
    const char* s_=A(c,'s'); const char* td=A(c,'t'); const char* dd=A(c,'d');
    if (!s_||!td||!dd) return -1;
    const char* p=A(c,'p'); bool byTime = !p || strcmp(p,"time")==0;
    int queryDay = mmddToDay(dd);
    FixStr sk(s_), tk(td);
    Vector<StationEntry>* sv = stationIndex.find(sk);
    Vector<StationEntry>* tv = stationIndex.find(tk);
    if (!sv||!tv){ sprintf(outbuf,"0"); return 0; }
    // collect pairs
    struct Rec { int trainIdx; int startDay; int fromIdx; int toIdx; long long leaveAbs, arriveAbs; int price; int seat; };
    Vector<Rec> recs;
    for (int i=0;i<sv->size();++i){
        StationEntry& e=(*sv)[i];
        // find matching entry in tv
        Train* t = trains[e.trainIdx]; if (!t||!t->released) continue;
        int fromIdx=e.stationIdx;
        // find toIdx for this train
        int toIdx=-1;
        for (int j=0;j<tv->size();++j){
            if ((*tv)[j].trainIdx==e.trainIdx && (*tv)[j].stationIdx>fromIdx){ toIdx=(*tv)[j].stationIdx; break; }
        }
        if (toIdx<0) continue;
        // compute start day: queryDay is day user departs from fromIdx. Train starts day D, leaves station fromIdx at D*1440 + leaveMin[fromIdx]. date = floor(that/1440)
        int leaveMin = t->leaveMin[fromIdx];
        int dayOffset = leaveMin/1440;
        int startDay = queryDay - dayOffset;
        if (startDay<t->saleStart||startDay>t->saleEnd) continue;
        Rec r;
        r.trainIdx=e.trainIdx; r.startDay=startDay; r.fromIdx=fromIdx; r.toIdx=toIdx;
        r.leaveAbs = (long long)startDay*1440 + leaveMin;
        r.arriveAbs = (long long)startDay*1440 + t->arriveMin[toIdx];
        r.price = t->prefixPrice[toIdx]-t->prefixPrice[fromIdx];
        r.seat = minSeats(t, startDay, fromIdx, toIdx);
        recs.push_back(r);
    }
    // sort recs
    int n=recs.size();
    // simple bubble sort for simplicity (may be slow but no algorithm lib)
    // use insertion sort
    for (int i=1;i<n;++i){
        Rec key=recs[i]; int j=i-1;
        while (j>=0){
            Rec& a=recs[j];
            bool cmpLess; // a > key?
            long long at = a.arriveAbs-a.leaveAbs, kt=key.arriveAbs-key.leaveAbs;
            if (byTime){
                if (at!=kt) cmpLess = at>kt;
                else cmpLess = strcmp(trains[a.trainIdx]->trainID, trains[key.trainIdx]->trainID)>0;
            } else {
                if (a.price!=key.price) cmpLess = a.price>key.price;
                else cmpLess = strcmp(trains[a.trainIdx]->trainID, trains[key.trainIdx]->trainID)>0;
            }
            if (!cmpLess) break;
            recs[j+1]=recs[j]; --j;
        }
        recs[j+1]=key;
    }
    char* w=outbuf;
    w += sprintf(w, "%d", n);
    for (int i=0;i<n;++i){
        Rec& r=recs[i]; Train* t=trains[r.trainIdx];
        char la[16], aa[16];
        absMinToStr(r.leaveAbs, la); absMinToStr(r.arriveAbs, aa);
        w += sprintf(w, "\n%s %s %s -> %s %s %d %d", t->trainID, t->stations[r.fromIdx], la, t->stations[r.toIdx], aa, r.price, r.seat);
    }
    *w=0;
    return 0;
}

static int cmd_query_transfer(const Cmd& c, char* outbuf){
    const char* s_=A(c,'s'); const char* td=A(c,'t'); const char* dd=A(c,'d');
    if (!s_||!td||!dd){ strcpy(outbuf,"0"); return 0; }
    const char* p=A(c,'p'); bool byTime = !p || strcmp(p,"time")==0;
    int queryDay = mmddToDay(dd);
    FixStr sk(s_), tk(td);
    Vector<StationEntry>* sv = stationIndex.find(sk);
    if (!sv){ strcpy(outbuf,"0"); return 0; }
    bool found=false;
    long long bestMetric=0;  // primary
    int bestPrice=0; long long bestTime=0;
    char bestTrain1[24]={0}, bestTrain2[24]={0};
    // best record data
    struct Leg { int trainIdx; int startDay; int fromIdx; int toIdx; long long leaveAbs, arriveAbs; int price; int seat; };
    Leg L1, L2; bool haveBest=false;
    for (int i=0;i<sv->size();++i){
        StationEntry& e=(*sv)[i];
        Train* t1 = trains[e.trainIdx]; if (!t1||!t1->released) continue;
        int fromIdx=e.stationIdx;
        int leaveMin = t1->leaveMin[fromIdx];
        int dayOffset = leaveMin/1440;
        int startDay1 = queryDay - dayOffset;
        if (startDay1<t1->saleStart||startDay1>t1->saleEnd) continue;
        // try each middle station s in (fromIdx, stationNum-1]
        for (int m=fromIdx+1;m<t1->stationNum;++m){
            long long arriveAtM = (long long)startDay1*1440 + t1->arriveMin[m];
            int price1 = t1->prefixPrice[m]-t1->prefixPrice[fromIdx];
            int seat1 = minSeats(t1, startDay1, fromIdx, m);
            FixStr mk(t1->stations[m]);
            Vector<StationEntry>* mv = stationIndex.find(mk);
            if (!mv) continue;
            FixStr tk2(td);
            Vector<StationEntry>* tv2 = stationIndex.find(tk2);
            if (!tv2) continue;
            // for each train at station m that != t1->trainID
            for (int j=0;j<mv->size();++j){
                StationEntry& e2=(*mv)[j];
                if (e2.trainIdx==e.trainIdx) continue;
                Train* t2 = trains[e2.trainIdx]; if (!t2||!t2->released) continue;
                int m2 = e2.stationIdx;
                // find end station on t2 after m2
                int toIdx=-1;
                for (int q=0;q<tv2->size();++q){
                    if ((*tv2)[q].trainIdx==e2.trainIdx && (*tv2)[q].stationIdx>m2){ toIdx=(*tv2)[q].stationIdx; break; }
                }
                if (toIdx<0) continue;
                // compute earliest startDay2 such that leave time at m2 >= arriveAtM
                int leaveMin2 = t2->leaveMin[m2];
                int day2Offset = leaveMin2/1440;
                // leaveAbs2 = startDay2*1440 + leaveMin2
                // need leaveAbs2 >= arriveAtM
                // startDay2 >= (arriveAtM - leaveMin2)/1440 ceil
                long long need = arriveAtM - leaveMin2;
                int startDay2;
                if (need<=0) startDay2 = (int)((need - 1439)/1440); // floor? we want smallest startDay2 such that startDay2*1440 >= need
                else startDay2 = (int)((need + 1439)/1440);
                if (startDay2 < t2->saleStart) startDay2 = t2->saleStart;
                if (startDay2 > t2->saleEnd) continue;
                long long leaveAbs2 = (long long)startDay2*1440 + leaveMin2;
                if (leaveAbs2 < arriveAtM) continue;
                long long arriveAbs2 = (long long)startDay2*1440 + t2->arriveMin[toIdx];
                int price2 = t2->prefixPrice[toIdx]-t2->prefixPrice[m2];
                int seat2 = minSeats(t2, startDay2, m2, toIdx);
                long long leaveAbs1 = (long long)startDay1*1440 + leaveMin;
                long long totalTime = arriveAbs2 - leaveAbs1;
                int totalPrice = price1+price2;
                bool better=false;
                if (!haveBest) better=true;
                else {
                    if (byTime){
                        if (totalTime<bestTime) better=true;
                        else if (totalTime==bestTime){
                            if (totalPrice<bestPrice) better=true;
                            else if (totalPrice==bestPrice){
                                int cmp = strcmp(t1->trainID, bestTrain1);
                                if (cmp<0) better=true;
                                else if (cmp==0 && strcmp(t2->trainID, bestTrain2)<0) better=true;
                            }
                        }
                    } else {
                        if (totalPrice<bestPrice) better=true;
                        else if (totalPrice==bestPrice){
                            if (totalTime<bestTime) better=true;
                            else if (totalTime==bestTime){
                                int cmp = strcmp(t1->trainID, bestTrain1);
                                if (cmp<0) better=true;
                                else if (cmp==0 && strcmp(t2->trainID, bestTrain2)<0) better=true;
                            }
                        }
                    }
                }
                if (better){
                    haveBest=true;
                    bestTime=totalTime; bestPrice=totalPrice;
                    strcpy(bestTrain1, t1->trainID); strcpy(bestTrain2, t2->trainID);
                    L1.trainIdx=e.trainIdx; L1.startDay=startDay1; L1.fromIdx=fromIdx; L1.toIdx=m;
                    L1.leaveAbs=leaveAbs1; L1.arriveAbs=arriveAtM; L1.price=price1; L1.seat=seat1;
                    L2.trainIdx=e2.trainIdx; L2.startDay=startDay2; L2.fromIdx=m2; L2.toIdx=toIdx;
                    L2.leaveAbs=leaveAbs2; L2.arriveAbs=arriveAbs2; L2.price=price2; L2.seat=seat2;
                }
            }
        }
    }
    if (!haveBest){ strcpy(outbuf,"0"); return 0; }
    char* w=outbuf;
    {
        Train* t=trains[L1.trainIdx];
        char la[16],aa[16]; absMinToStr(L1.leaveAbs,la); absMinToStr(L1.arriveAbs,aa);
        w += sprintf(w,"%s %s %s -> %s %s %d %d\n", t->trainID, t->stations[L1.fromIdx], la, t->stations[L1.toIdx], aa, L1.price, L1.seat);
    }
    {
        Train* t=trains[L2.trainIdx];
        char la[16],aa[16]; absMinToStr(L2.leaveAbs,la); absMinToStr(L2.arriveAbs,aa);
        w += sprintf(w,"%s %s %s -> %s %s %d %d", t->trainID, t->stations[L2.fromIdx], la, t->stations[L2.toIdx], aa, L2.price, L2.seat);
    }
    *w=0;
    return 1; // signal output
}

static int cmd_buy_ticket(const Cmd& c, char* outbuf){
    const char* u=A(c,'u'); const char* i_=A(c,'i'); const char* d=A(c,'d'); const char* ns=A(c,'n');
    const char* f=A(c,'f'); const char* tg=A(c,'t'); const char* q=A(c,'q');
    bool queueIfFull = q && strcmp(q,"true")==0;
    if (!u||!i_||!d||!ns||!f||!tg) return -1;
    FixStr uk(u);
    if (!loggedIn.find(uk)) return -1;
    int* ui=userMap.find(uk); if (!ui) return -1;
    FixStr ik(i_);
    int* ti=trainMap.find(ik); if (!ti||!trains[*ti]||!trains[*ti]->released) return -1;
    Train* t=trains[*ti];
    int num=parseInt(ns);
    if (num<=0) return -1;
    if (num>t->seatNum) return -1;
    int queryDay=mmddToDay(d);
    // find fromIdx and toIdx
    int fromIdx=-1, toIdx=-1;
    for (int k=0;k<t->stationNum;++k){
        if (fromIdx<0 && strcmp(t->stations[k],f)==0) fromIdx=k;
        if (fromIdx>=0 && strcmp(t->stations[k],tg)==0){ toIdx=k; break; }
    }
    if (fromIdx<0||toIdx<0||toIdx<=fromIdx) return -1;
    int leaveMin=t->leaveMin[fromIdx]; int dayOffset=leaveMin/1440;
    int startDay=queryDay-dayOffset;
    if (startDay<t->saleStart||startDay>t->saleEnd) return -1;
    int avail = minSeats(t, startDay, fromIdx, toIdx);
    int price = (t->prefixPrice[toIdx]-t->prefixPrice[fromIdx])*num;
    Order od; memset(&od,0,sizeof(od));
    od.trainIdx=*ti; strncpy(od.trainID, t->trainID, 23);
    strncpy(od.fromSt, t->stations[fromIdx], 15);
    strncpy(od.toSt, t->stations[toIdx], 15);
    od.fromIdx=fromIdx; od.toIdx=toIdx; od.day=startDay; od.num=num;
    od.price = t->prefixPrice[toIdx]-t->prefixPrice[fromIdx];
    od.leaveAbs=(long long)startDay*1440+leaveMin;
    od.arriveAbs=(long long)startDay*1440+t->arriveMin[toIdx];
    od.orderSeq=globalOrderSeq++;
    if (avail>=num){
        applySeats(t, startDay, fromIdx, toIdx, -num);
        od.status=0;
        allOrders[*ui].push_back(od);
        sprintf(outbuf, "%d", price);
        return 0;
    }
    if (queueIfFull){
        od.status=1;
        allOrders[*ui].push_back(od);
        Pending pd; pd.userIdx=*ui; pd.orderIdx=allOrders[*ui].size()-1; pd.fromIdx=fromIdx; pd.toIdx=toIdx; pd.num=num;
        int dayIdx=startDay-t->saleStart;
        pendingLists[*ti][dayIdx].push_back(pd);
        strcpy(outbuf, "queue");
        return 0;
    }
    return -1;
}

static int cmd_query_order(const Cmd& c, char* outbuf){
    const char* u=A(c,'u'); if (!u) return -1;
    FixStr uk(u);
    if (!loggedIn.find(uk)) return -1;
    int* ui=userMap.find(uk); if (!ui) return -1;
    Vector<Order>& ords = allOrders[*ui];
    int n=ords.size();
    char* w=outbuf;
    w += sprintf(w,"%d", n);
    for (int i=n-1;i>=0;--i){
        Order& o=ords[i];
        const char* st = o.status==0?"success":(o.status==1?"pending":"refunded");
        char la[16],aa[16]; absMinToStr(o.leaveAbs,la); absMinToStr(o.arriveAbs,aa);
        w += sprintf(w,"\n[%s] %s %s %s -> %s %s %d %d", st, o.trainID, o.fromSt, la, o.toSt, aa, o.price, o.num);
    }
    *w=0;
    return 0;
}

static void tryFulfillPending(int trainIdx, int day){
    Train* t = trains[trainIdx]; if (!t) return;
    int dayIdx = day - t->saleStart;
    Vector<Pending>& pl = pendingLists[trainIdx][dayIdx];
    for (int i=0;i<pl.size();){
        Pending& p=pl[i];
        int avail = minSeats(t, day, p.fromIdx, p.toIdx);
        if (avail>=p.num){
            applySeats(t, day, p.fromIdx, p.toIdx, -p.num);
            Order& o = allOrders[p.userIdx][p.orderIdx];
            o.status=0;
            pl.erase(i);
        } else ++i;
    }
}

static int cmd_refund_ticket(const Cmd& c){
    const char* u=A(c,'u'); if (!u) return -1;
    FixStr uk(u);
    if (!loggedIn.find(uk)) return -1;
    int* ui=userMap.find(uk); if (!ui) return -1;
    int n = A(c,'n')?parseInt(A(c,'n')):1;
    Vector<Order>& ords = allOrders[*ui];
    if (n<1 || n>ords.size()) return -1;
    int idx = ords.size()-n;
    Order& o = ords[idx];
    if (o.status==2) return -1;
    if (o.status==0){
        // refund seats
        applySeats(trains[o.trainIdx], o.day, o.fromIdx, o.toIdx, +o.num);
        o.status=2;
        tryFulfillPending(o.trainIdx, o.day);
    } else if (o.status==1){
        // remove from pending list
        int dayIdx = o.day - trains[o.trainIdx]->saleStart;
        Vector<Pending>& pl = pendingLists[o.trainIdx][dayIdx];
        for (int i=0;i<pl.size();++i){
            if (pl[i].userIdx==*ui && pl[i].orderIdx==idx){ pl.erase(i); break; }
        }
        o.status=2;
    }
    return 0;
}

static int cmd_clean(){
    // Clear everything
    userMap.clear(); users.clear(); allOrders.clear(); loggedIn.clear();
    trainMap.clear();
    for (int i=0;i<trains.size();++i){ if (trains[i]){ if (trains[i]->seats) delete[] trains[i]->seats; delete trains[i]; } }
    trains.clear();
    stationIndex.clear();
    pendingLists.clear();
    globalOrderSeq=0;
    return 0;
}

// ------------- Main -------------
int main(){
    static char line[65536];
    static char out[1<<20];
    while (fgets(line, sizeof(line), stdin)){
        // print timestamp prefix as required? No, README doesn't specify timestamp
        Cmd c;
        if (!parseCmd(line, c)) continue;
        if (c.name[0]==0) continue;
        // README examples show outputs without [timestamp] prefix; many ticketing variants use [timestamp].
        // But here: no timestamp in examples. Follow README.
        out[0]=0;
        if (strcmp(c.name,"add_user")==0){ printf("%d\n", cmd_add_user(c)); }
        else if (strcmp(c.name,"login")==0){ printf("%d\n", cmd_login(c)); }
        else if (strcmp(c.name,"logout")==0){ printf("%d\n", cmd_logout(c)); }
        else if (strcmp(c.name,"query_profile")==0){ int r=cmd_query_profile(c,out); if (r==0) printf("%s\n",out); else printf("-1\n"); }
        else if (strcmp(c.name,"modify_profile")==0){ int r=cmd_modify_profile(c,out); if (r==0) printf("%s\n",out); else printf("-1\n"); }
        else if (strcmp(c.name,"add_train")==0){ printf("%d\n", cmd_add_train(c)); }
        else if (strcmp(c.name,"release_train")==0){ printf("%d\n", cmd_release_train(c)); }
        else if (strcmp(c.name,"delete_train")==0){ printf("%d\n", cmd_delete_train(c)); }
        else if (strcmp(c.name,"query_train")==0){ int r=cmd_query_train(c,out); if (r==0) printf("%s\n",out); else printf("-1\n"); }
        else if (strcmp(c.name,"query_ticket")==0){ cmd_query_ticket(c,out); printf("%s\n",out); }
        else if (strcmp(c.name,"query_transfer")==0){ int r=cmd_query_transfer(c,out); if (r==1) printf("%s\n",out); else printf("0\n"); }
        else if (strcmp(c.name,"buy_ticket")==0){ int r=cmd_buy_ticket(c,out); if (r==0) printf("%s\n",out); else printf("-1\n"); }
        else if (strcmp(c.name,"query_order")==0){ int r=cmd_query_order(c,out); if (r==0) printf("%s\n",out); else printf("-1\n"); }
        else if (strcmp(c.name,"refund_ticket")==0){ printf("%d\n", cmd_refund_ticket(c)); }
        else if (strcmp(c.name,"clean")==0){ printf("%d\n", cmd_clean()); }
        else if (strcmp(c.name,"exit")==0){ printf("bye\n"); break; }
    }
    return 0;
}
