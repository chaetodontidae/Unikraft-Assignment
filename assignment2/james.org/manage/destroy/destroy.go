package destroy

import "james.org/manage/query"

func DestroyVm(vm string) (bool, error) {
	conn, dom, err := query.QueryDomain(vm)
	if err != nil {
		return false, err
	}
	if conn == nil || dom == nil {
		return false, nil
	}
	dom.Destroy()
	defer conn.Close()
	defer dom.Free()
	return true, nil
}
