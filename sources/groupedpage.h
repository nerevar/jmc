#ifndef _GROUPED_PAGE_H_

#define _GROUPED_PAGE_H_

class CGroupedPage : public CPropertyPage {
public:
    CGroupedPage(UINT nIDTemplate, UINT nIDCaption = 0);
    static CMap<wchar_t*, wchar_t*, CGroupedPage*, CGroupedPage*> m_Pages;
    void AddPage(wchar_t* key, CGroupedPage* pg) ;
    
    virtual void GroupListChanged() = 0;

    static void NotifyAll();
};


#endif //_GROUPED_PAGE_H_