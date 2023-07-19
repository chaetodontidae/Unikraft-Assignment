package main

import (
	"context"
	"fmt"
	"github.com/gin-gonic/gin"
	"io/ioutil"
	"james.org/manage/create"
	"james.org/manage/destroy"
	"james.org/manage/pause"
	"james.org/manage/query"
	"james.org/manage/snapshot"
	"log"
	"net/http"
	"time"
)

func registRouter() (*gin.Engine, chan bool) {
	r := gin.Default()
	ch := make(chan bool, 1)
	r.DELETE("/vm", func(c *gin.Context) {
		name, existed := c.GetQuery("name")
		if !existed {
			c.String(200, "require parameter name to destroy vm")
			return
		}
		res, err := destroy.DestroyVm(name)
		if err != nil {
			c.JSON(200, err)
		} else {
			if !res {
				c.String(200, "the vm does not exist, maybe it has been destroyed")
			} else {
				c.String(200, "successfully destroy vm:"+name)
			}
		}
	})
	r.POST("/vm", func(c *gin.Context) {
		xml_name, existed := c.GetQuery("xmlName")
		if !existed {
			c.String(200, "require parameter xmlName to create vm")
			return
		}
		err := create.CreateVm(xml_name)
		if err != nil {
			c.JSON(200, err)
		} else {
			c.String(200, "successfully create vm")
		}
	})
	r.GET("/vms", func(c *gin.Context) {
		arr := query.ShowVms()
		if arr != nil {
			c.JSON(200, arr)
		} else {
			c.String(200, "query failed")
		}
	})
	r.GET("/shutdown", func(c *gin.Context) {
		ch <- true
		c.String(200, "receive shutdown request")
	})
	r.POST("/vm/suspend", func(c *gin.Context) {
		name, existed := c.GetQuery("name")
		if !existed {
			c.String(200, "require parameter name to suspend vm")
			return
		}
		res, err := pause.SuspendVm(name)
		if err != nil {
			c.JSON(200, err)
			return
		}
		if !res {
			c.String(200, "the vm does not exist, maybe it has been destroyed")
		} else {
			c.String(200, "successfully suspend the vm:"+name)
		}
	})
	r.POST("/vm/resume", func(c *gin.Context) {
		name, existed := c.GetQuery("name")
		if !existed {
			c.String(200, "require parameter name to resume vm")
			return
		}
		res, err := pause.ResumeVm(name)
		if err != nil {
			c.JSON(200, err)
			return
		}
		if !res {
			c.String(200, "the vm does not exist, maybe it has been destroyed")
		} else {
			c.String(200, "successfully suspend the vm:"+name)
		}
	})
	r.POST("/vm/snapshot", func(c *gin.Context) {
		name, existed := c.GetQuery("name")
		if !existed {
			c.String(200, "require parameter name to create vm")
			return
		}
		xmlContent, err := ioutil.ReadAll(c.Request.Body)
		if err != nil {
			c.JSON(200, err)
			return
		}
		res, err := snapshot.CreateSnapshot(name, string(xmlContent))
		if err != nil {
			c.JSON(200, err)
			return
		}
		if !res {
			c.String(200, "the vm does not exist, maybe it has been destroyed")
		} else {
			c.String(200, "successfully create snapshot for the vm:"+name)
		}
	})
	r.POST("/vm/snapshot/revert", func(c *gin.Context) {
		name, existed := c.GetQuery("name")
		if !existed {
			c.String(200, "require parameter name to create vm")
			return
		}
		res, err := snapshot.RevertToLastSnapshot(name)
		if err != nil {
			c.JSON(200, err)
			return
		}
		if !res {
			c.String(200, "the snapshot does not exist")
		} else {
			c.String(200, "successfully revert snapshot for the vm:"+name)
		}
	})
	return r, ch
}

func main() {
	router, shut_chan := registRouter()

	srv := &http.Server{
		Addr:    ":9000",
		Handler: router,
	}

	go func() {
		if err := srv.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			fmt.Printf("listen: %s\n", err)
		}
	}()

	<-shut_chan
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()
	if err := srv.Shutdown(ctx); err != nil {
		fmt.Printf("Server Shutdown:%v", err)
	}

	select {
	case <-ctx.Done():
		log.Println("timeout of 3 seconds.")
	}
	log.Println("Server exiting")

}
