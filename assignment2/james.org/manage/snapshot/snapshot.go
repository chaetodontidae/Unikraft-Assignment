package snapshot

import (
	"fmt"
	"james.org/manage/query"
	"libvirt.org/go/libvirt"
)

func GetLatestSnapshot(vm string) (*libvirt.Connect, *libvirt.DomainSnapshot, error) {
	conn, dom, err := query.QueryDomain(vm)
	if err != nil {
		return nil, nil, err
	}
	if conn == nil || dom == nil {
		return nil, nil, nil
	}
	snapshot, err := dom.SnapshotCurrent(0)
	if err != nil {
		dom.Free()
		conn.Close()
		return nil, nil, err
	}
	dom.Free()
	fmt.Printf("snapshot is null:%v", snapshot == nil)
	return conn, snapshot, nil
}

func RevertToLastSnapshot(vm string) (bool, error) {
	conn, snapshot, err := GetLatestSnapshot(vm)
	if err != nil {
		return false, err
	}
	if conn == nil || snapshot == nil {
		return false, nil
	}
	snapshot.RevertToSnapshot(0)
	snapshot.Free()
	conn.Close()
	return true, nil
}

func CreateSnapshot(vm string, xmlContent string) (bool, error) {
	conn, dom, err := query.QueryDomain(vm)
	if err != nil {
		return false, err
	}
	if conn == nil || dom == nil {
		return false, nil
	}
	snapshot, err := dom.CreateSnapshotXML(xmlContent, 0)
	defer dom.Free()
	defer conn.Close()
	if err != nil {
		return false, err
	} else {
		snapshot.Free()
	}
	return true, nil
}
