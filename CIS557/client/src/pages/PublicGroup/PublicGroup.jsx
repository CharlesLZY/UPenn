import React, { useState, useEffect } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import { makeStyles } from '@mui/styles';
import {
  Container, Typography, Button, Paper, IconButton, Fab, Modal, Dialog,
} from '@mui/material';

import Edit from '@mui/icons-material/Edit';
import PersonAdd from '@mui/icons-material/PersonAdd';
import AssignmentInd from '@mui/icons-material/AssignmentInd';
import ExitToApp from '@mui/icons-material/ExitToApp';

import Layout from '../../components/Layout/Layout';
import JoinButton from '../../components/PublicGroup/JoinButton';
import PostBar from '../../components/PublicGroup/PostBar';
import MemberBar from '../../components/PublicGroup/MemberBar';
import InviteDialog from '../../components/PublicGroup/InviteDialog';

import * as fetchUser from '../../utils/fetchUser';
import * as fetchGroup from '../../utils/fetchPublicGroup';

import * as utils from '../../utils/utils';

const useStyles = makeStyles({
  banner: {
    display: 'flex',
    justifyContent: 'center',
    height: 80,
    width: 1170,
  },
  groupName: {
    marginTop: 'auto',
    marginBottom: 'auto',
  },
  joinButtonArea: {
    marginTop: 'auto',
    marginBottom: 'auto',
    marginLeft: 30,
  },

  postSection: {
    margin: '25px',
  },
  postSectionBG: {
    marginTop: '10px',
    width: '850px',
  },
  body: {
    display: 'flex',
    width: 1170,
  },
  actionSectionBG: {
    marginTop: '10px',
    marginLeft: '20px',

  },
  actionSection: {
    width: '300px',
    display: 'flex',
  },

  right: {
    display: 'inlined',
  },

  modal: {
    paddingTop: 50,
    width: 400,
    height: 120,
    textAlign: 'center',
    margin: 0,
    top: '35%',
    right: document.body.clientWidth / 2 - 200,
    bottom: 'auto',
    left: 'auto',
    position: 'fixed',
  },

});

export default function PublicGroup() {
  const classes = useStyles();
  const navigate = useNavigate();
  const params = useParams();
  const { groupID } = params;
  const userID = sessionStorage.getItem('userID');

  const [group, setGroup] = useState(null);
  const [userInfo, setUserInfo] = useState(null);
  const [posts, setPosts] = useState(null);

  const [exitModal, setExitModal] = useState(false);
  const [inviteModal, setInviteModal] = useState(false);

  useEffect(async () => {
    const data = await fetchGroup.fetchGroup(groupID);
    if (data === null) {
      navigate('/wrongpage');
    } else {
      setGroup(data);
      if (userID) {
        const user = await fetchUser.fetchUser(userID);
        setUserInfo(user);
      }
      const allPosts = await fetchGroup.fetchGroupPost(groupID, userID);
      setPosts(allPosts);
    }
  }, []);

  const handleClickAdd = () => {
    navigate(`/createpublicpost/${groupID}`);
  };

  const handleClickAdmin = () => {
    navigate(`/publicgroupadmin/${groupID}`);
  };

  const handleExit = async () => {
    await fetchGroup.exitGroup(groupID, userID);
    navigate(0);
  };

  function actionBar(group) {
    return (
      <Paper className={classes.actionSectionBG}>
        <div className={classes.actionSection}>

          <IconButton size="medium" sx={{ marginLeft: '30px' }} onClick={handleClickAdd}><Edit /></IconButton>
          <IconButton size="medium" sx={{ marginLeft: '25px' }} onClick={() => setInviteModal(true)}><PersonAdd /></IconButton>
          {group && group.admins.indexOf(userID) >= 0 && <IconButton size="medium" sx={{ marginLeft: '25px' }} onClick={handleClickAdmin}><AssignmentInd /></IconButton>}
          {group && group.creator !== userID && <IconButton size="medium" sx={{ marginLeft: '25px' }} onClick={() => setExitModal(true)}><ExitToApp /></IconButton>}

        </div>
      </Paper>
    );
  }

  return (
    <Layout>
      <Container>
        <Paper className={classes.banner}>
          <div className={classes.groupName}>
            <Typography variant="h3" color="secondary">
              {groupID}
            </Typography>
          </div>
          <div className={classes.joinButtonArea}>
            <JoinButton userInfo={userInfo} group={group} />
          </div>
        </Paper>
        <div className={classes.body}>
          <div>
            {posts && <PostBar posts={posts} userInfo={userInfo} group={group} />}
          </div>
          <div className={classes.right}>
            <div>
              {utils.checkUserInPublicGroup(userInfo, groupID) && actionBar(group)}
            </div>
            <div>
              {group && <MemberBar group={group} userID={userID} />}
            </div>
          </div>
        </div>
        {utils.checkUserInPublicGroup(userInfo, groupID)
                    && (
                    <Fab
                      color="secondary"
                      onClick={handleClickAdd}
                      style={{
                        margin: 0,
                        top: 'auto',
                        right: document.body.clientWidth / 2 - 280,
                        bottom: 40,
                        left: 'auto',
                        position: 'fixed',
                      }}
                    >
                      <Edit />
                    </Fab>
                    )}
        <Modal open={exitModal} onClose={() => setExitModal(false)}>
          <Paper className={classes.modal}>
            <Typography variant="h6">
              {`Do you want to exit ${groupID}`}
            </Typography>
            <Button sx={{ margin: '20px', fontSize: 20 }} color="secondary" onClick={handleExit}>Yes</Button>
            <Button sx={{ margin: '20px', fontSize: 20 }} color="secondary" onClick={() => setExitModal(false)}>No</Button>
          </Paper>
        </Modal>
        <Dialog open={inviteModal} onClose={() => setInviteModal(false)}>
          <InviteDialog group={group} userID={userID} />
        </Dialog>
      </Container>
    </Layout>
  );
}
