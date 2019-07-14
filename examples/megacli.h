/**
 * @file megaclient.cpp
 * @brief sample application, interactive GNU Readline CLI
 *
 * (c) 2013 by Mega Limited, Wellsford, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

using namespace mega;

extern MegaClient* client;
extern MegaClient* clientFolder;

extern void megacli();

extern void term_init();
extern void term_restore();
extern void term_echo(int);

extern void read_pw_char(char*, int, int*, char**);

typedef list<struct AppFile*> appfile_list;

struct AppFile : public File
{
    // app-internal sequence number for queue management
    int seqno;

    void progress();

    appfile_list::iterator appxfer_it;

    AppFile();
};

// application-managed GET and PUT queues (only pending and active files)
extern appfile_list appxferq[2];

struct AppFileGet : public AppFile
{
    void start();
    void update();
    void completed(Transfer*, LocalNode*);

    AppFileGet(Node*, handle = UNDEF, byte* = NULL, m_off_t = -1, m_time_t = 0, string* = NULL, string* = NULL, const string& targetfolder = "");
    ~AppFileGet();
};

struct AppFilePut : public AppFile
{
    void start();
    void update();
    void completed(Transfer*, LocalNode*);

    void displayname(string*);

    AppFilePut(string*, handle, const char*);
    ~AppFilePut();
};

struct AppReadContext
{
    SymmCipher key;
};

class TreeProcListOutShares : public TreeProc
{
public:
    void proc(MegaClient*, Node*);
};

struct DemoApp : public MegaApp
{
    FileAccess* newfile();

    void request_error(error) override;
    
    void request_response_progress(m_off_t, m_off_t) override;
    
    void prelogin_result(int version, string* email, string *salt, error e) override;
    void login_result(error) override;
    void multifactorauthdisable_result(error) override;
    void multifactorauthsetup_result(string *code, error e) override;
    void multifactorauthcheck_result(int enabled) override;

    void ephemeral_result(error) override;
    void ephemeral_result(handle, const byte*) override;
    void cancelsignup_result(error) override;

    void whyamiblocked_result(error) override;

    void sendsignuplink_result(error) override;
    void querysignuplink_result(error) override;
    void querysignuplink_result(handle, const char*, const char*, const byte*, const byte*, const byte*, size_t) override;
    void confirmsignuplink_result(error) override;
    void setkeypair_result(error) override;

    void getrecoverylink_result(error) override;
    void queryrecoverylink_result(error) override;
    void queryrecoverylink_result(int type, const char *email, const char *ip, time_t ts, handle uh, const vector<string> *emails) override;
    void getprivatekey_result(error,  const byte *privk, const size_t len_privk) override;
    void confirmrecoverylink_result(error) override;
    void confirmcancellink_result(error) override;
    void validatepassword_result(error) override;
    void getemaillink_result(error) override;
    void confirmemaillink_result(error) override;

    void users_updated(User**, int) override;
    void useralerts_updated(UserAlert::Base** ua, int count) override;
    void nodes_updated(Node**, int) override;
    void pcrs_updated(PendingContactRequest**, int) override;
    void nodes_current() override;
    void account_updated() override;
    void notify_confirmation(const char *email) override;

#ifdef ENABLE_CHAT
    void chatcreate_result(TextChat *, error) override;
    void chatinvite_result(error) override;
    void chatremove_result(error) override;
    void chaturl_result(string *, error) override;
    void chatgrantaccess_result(error) override;
    void chatremoveaccess_result(error) override;
    virtual void chatupdatepermissions_result(error) override;
    virtual void chattruncate_result(error) override;
    virtual void chatsettitle_result(error) override;
    virtual void chatpresenceurl_result(string *, error) override;
    void chatlink_result(handle, error) override;
    void chatlinkclose_result(error) override;
    void chatlinkurl_result(handle, int, string*, string*, int, m_time_t, error) override;
    void chatlinkjoin_result(error) override;

    void chats_updated(textchat_map*, int) override;

    static void printChatInformation(TextChat *);
    static string getPrivilegeString(privilege_t priv);

    void richlinkrequest_result(string*, error) override;
#endif

    void setattr_result(handle, error) override;
    void rename_result(handle, error) override;
    void unlink_result(handle, error) override;

    void fetchnodes_result(error) override;

    void putnodes_result(error, targettype_t, NewNode*) override;

    void share_result(error) override;
    void share_result(int, error) override;

    void setpcr_result(handle, error, opcactions_t) override;
    void updatepcr_result(error, ipcactions_t) override;

    void fa_complete(handle, fatype, const char*, uint32_t) override;
    int fa_failed(handle, fatype, int, error) override;

    void putfa_result(handle, fatype, error) override;
    void putfa_result(handle, fatype, const char*) override;

    void removecontact_result(error) override;
    void putua_result(error) override;
    void getua_result(error) override;
    void getua_result(byte*, unsigned, attr_t) override;
    void getua_result(TLVstore *, attr_t) override;
#ifdef DEBUG
    void delua_result(error) override;
#endif

    void querytransferquota_result(int) override;

    void account_details(AccountDetails*, bool, bool, bool, bool, bool, bool) override;
    void account_details(AccountDetails*, error) override;

    // sessionid is undef if all sessions except the current were killed
    void sessions_killed(handle sessionid, error e) override;

    void exportnode_result(error) override;
    void exportnode_result(handle, handle) override;

    void openfilelink_result(error) override;
    void openfilelink_result(handle, const byte*, m_off_t, string*, string*, int) override;

    void folderlinkinfo_result(error, handle, handle, string *, string*, m_off_t, uint32_t, uint32_t, m_off_t, uint32_t) override;

    void checkfile_result(handle, error) override;
    void checkfile_result(handle, error, byte*, m_off_t, m_time_t, m_time_t, string*, string*, string*) override;

    dstime pread_failure(error, int, void*, dstime) override;
    bool pread_data(byte*, m_off_t, m_off_t, m_off_t, m_off_t, void*) override;

    void transfer_added(Transfer*) override;
    void transfer_removed(Transfer*) override;
    void transfer_prepare(Transfer*) override;
    void transfer_failed(Transfer*, error, dstime) override;
    void transfer_update(Transfer*) override;
    void transfer_complete(Transfer*) override;

#ifdef ENABLE_SYNC
    void syncupdate_state(Sync*, syncstate_t) override;
    void syncupdate_scanning(bool) override;
    void syncupdate_local_folder_addition(Sync*, LocalNode*, const char*) override;
    void syncupdate_local_folder_deletion(Sync* , LocalNode*) override;
    void syncupdate_local_file_addition(Sync*, LocalNode*, const char*) override;
    void syncupdate_local_file_deletion(Sync*, LocalNode*) override;
    void syncupdate_local_file_change(Sync*, LocalNode*, const char*) override;
    void syncupdate_local_move(Sync*, LocalNode*, const char*) override;
    void syncupdate_local_lockretry(bool) override;
    void syncupdate_get(Sync*, Node*, const char*) override;
    void syncupdate_put(Sync*, LocalNode*, const char*) override;
    void syncupdate_remote_file_addition(Sync*, Node*) override;
    void syncupdate_remote_file_deletion(Sync*, Node*) override;
    void syncupdate_remote_folder_addition(Sync*, Node*) override;
    void syncupdate_remote_folder_deletion(Sync*, Node*) override;
    void syncupdate_remote_copy(Sync*, const char*) override;
    void syncupdate_remote_move(Sync*, Node*, Node*) override;
    void syncupdate_remote_rename(Sync*, Node*, const char*) override;
    void syncupdate_treestate(LocalNode*) override;

    bool sync_syncable(Sync*, const char*, string*, Node*) override;
    bool sync_syncable(Sync*, const char*, string*) override;
#endif

    void changepw_result(error) override;

    void userattr_update(User*, int, const char*) override;

    void enumeratequotaitems_result(handle, unsigned, unsigned, unsigned, unsigned, unsigned, const char*, const char*, const char*, const char*) override;
    void enumeratequotaitems_result(error) override;
    void additem_result(error) override;
    void checkout_result(const char*, error) override;

    void getmegaachievements_result(AchievementsDetails*, error) override;
    void getwelcomepdf_result(handle, string*, error) override;

    void contactlinkcreate_result(error, handle) override;
    void contactlinkquery_result(error, handle, string*, string*, string*, string*) override;
    void contactlinkdelete_result(error) override;

    void reload(const char*) override;
    void clearing() override;

    void notify_retry(dstime, retryreason_t) override;
};

struct DemoAppFolder : public DemoApp
{
    void login_result(error);
    void fetchnodes_result(error);

    void nodes_updated(Node **, int);
    void users_updated(User**, int) {}
    void pcrs_updated(PendingContactRequest**, int) {}
};
