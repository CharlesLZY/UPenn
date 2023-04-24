import { makeStyles } from '@mui/styles';
import { Container, Paper, Typography } from '@mui/material';

const useStyles = makeStyles((theme) => ({
  root: {
    display: 'inlined',
  },

  paper: {
    display: 'inlined',
    textAlign: 'center',
    width: '850px',
    height: 150,
    marginLeft: 'auto',
    marginRight: 'auto',
    marginTop: '10vw',
    paddingTop: 100,
    paddingBottom: 100,
  },
  padding: {
    height: '20vw',
  },

}));

export default function ForgetPassword() {
  const classes = useStyles();

  return (

    <Container className={classes.root}>
      <Paper className={classes.paper} elevation={3}>
        <Typography
          variant="h4"
        >
          If you forget the password, please contact us.
        </Typography>
        <br />
        <Typography
          variant="h3"
          color="secondary"
        >
          charlesliangzhiyuan@gmail.com
        </Typography>
      </Paper>
      <div className={classes.padding} />
    </Container>

  );
}
