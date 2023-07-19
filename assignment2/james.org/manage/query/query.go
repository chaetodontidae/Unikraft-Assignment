package query

import (
	"fmt"
	"libvirt.org/go/libvirt"
)

func QueryDomain(vm string) (*libvirt.Connect, *libvirt.Domain, error) {
	conn, err := libvirt.NewConnect("qemu:///system")
	if err != nil {
		fmt.Printf("exception happens:%v\n", err)
		return nil, nil, err
	}

	doms, err := conn.ListAllDomains(libvirt.CONNECT_LIST_DOMAINS_ACTIVE)
	if err != nil {
		fmt.Printf("exception happens:%v\n", err)
		conn.Close()
		return nil, nil, err
	}
	var ptr *libvirt.Domain
	ptr = nil
	for _, dom := range doms {
		name, err := dom.GetName()
		if err == nil && name == vm {
			ptr = &dom
		} else {
			dom.Free()
		}
	}
	if ptr == nil {
		conn.Close()
		return nil, nil, nil
	}
	return conn, ptr, nil
}

func ShowVms() []string {
	conn, err := libvirt.NewConnect("qemu:///system")
	if err != nil {
		fmt.Printf("exception happens:%v\n", err)
		return nil
	}
	defer conn.Close()

	doms, err := conn.ListAllDomains(libvirt.CONNECT_LIST_DOMAINS_ACTIVE)
	if err != nil {
		fmt.Printf("exception happens:%v\n", err)
		return nil
	}
	names := make([]string, 0)
	for _, dom := range doms {
		name, err := dom.GetName()
		if err == nil {
			names = append(names, name)
		}
		dom.Free()
	}
	return names
}
