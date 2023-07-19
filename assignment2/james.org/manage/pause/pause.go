package pause

import "james.org/manage/query"

func SuspendVm(vm string) (bool, error) {
	conn, dom, err := query.QueryDomain(vm)
	if err != nil {
		return false, err
	}
	if conn == nil || dom == nil {
		return false, nil
	}
	dom.Suspend()
	defer conn.Close()
	defer dom.Free()
	return true, nil
}

func ResumeVm(vm string) (bool, error) {
	conn, dom, err := query.QueryDomain(vm)
	if err != nil {
		return false, err
	}
	if conn == nil || dom == nil {
		return false, nil
	}
	dom.Resume()
	defer conn.Close()
	defer dom.Free()
	return true, nil
}
