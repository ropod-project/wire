/************************************************************************
 *  Copyright (C) 2012 Eindhoven University of Technology (TU/e).       *
 *  All rights reserved.                                                *
 ************************************************************************
 *  Redistribution and use in source and binary forms, with or without  *
 *  modification, are permitted provided that the following conditions  *
 *  are met:                                                            *
 *                                                                      *
 *      1.  Redistributions of source code must retain the above        *
 *          copyright notice, this list of conditions and the following *
 *          disclaimer.                                                 *
 *                                                                      *
 *      2.  Redistributions in binary form must reproduce the above     *
 *          copyright notice, this list of conditions and the following *
 *          disclaimer in the documentation and/or other materials      *
 *          provided with the distribution.                             *
 *                                                                      *
 *  THIS SOFTWARE IS PROVIDED BY TU/e "AS IS" AND ANY EXPRESS OR        *
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED      *
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  *
 *  ARE DISCLAIMED. IN NO EVENT SHALL TU/e OR CONTRIBUTORS BE LIABLE    *
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR        *
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT   *
 *  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;     *
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF       *
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT           *
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE   *
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH    *
 *  DAMAGE.                                                             *
 *                                                                      *
 *  The views and conclusions contained in the software and             *
 *  documentation are those of the authors and should not be            *
 *  interpreted as representing official policies, either expressed or  *
 *  implied, of TU/e.                                                   *
 ************************************************************************/

#include "KalmanFilter.h"

//#include <limits> // TEMP

KalmanFilter::KalmanFilter(int dim)
: meas_dim_(dim), state_dim_(dim * 2), G_(dim * 2), a_max_(0) {
       G_small_ = std::make_shared<pbl::Gaussian>(dim);
}

KalmanFilter::KalmanFilter(const KalmanFilter& orig)
: meas_dim_(orig.meas_dim_), state_dim_(orig.state_dim_), G_(orig.G_), H_(orig.H_), a_max_(orig.a_max_) {
        G_small_ = std::make_shared<pbl::Gaussian>(*(orig.G_small_));
}

KalmanFilter::~KalmanFilter() {
}

KalmanFilter* KalmanFilter::clone() const {
	return new KalmanFilter(*this);
}

//void KalmanFilter::init(const pbl::Gaussian& z) {
//	H_ = Eigen::MatrixXd::Identity(meas_dim_, state_dim_);

//	G_.setMean(H_.transpose() * z.getMean());
//	G_.setCovariance(H_.transpose() * z.getCovariance() * H_);
//
//	G_small_.setMean(z.getMean());;
//	G_small_.setCovariance(z.getCovariance());;
//}

void KalmanFilter::init(std::shared_ptr<const pbl::Gaussian> z) {
        H_ = arma::eye(meas_dim_, state_dim_);

        G_.setMean(H_.t() * z->getMean());
        G_.setCovariance(H_.t() * z->getCovariance() * H_);

        G_small_->setMean(z->getMean());;
        G_small_->setCovariance(z->getCovariance());
         std::string s = this->toString();
        std::cout << "KF init: " << s << std::endl;
}

void KalmanFilter::propagate(const double& dt) {
       std::string s = this->toString();
       std::cout << "KF propagate start: " << s << std::endl;
       
       std::cout << "dt = " << dt << std::endl;
	if (a_max_ > 0) {
		//		const pbl::Vector& x = G_.getMean();
		//		const pbl::Matrix& P = G_.getCovariance();

		// set state transition matrix
		//Eigen::MatrixXd F;
                //F = Eigen::MatrixXd::Identity(state_dim_, state_dim_);
                pbl::Matrix F = arma::eye(state_dim_, state_dim_);
                
                std::cout << "F = " << F << std::endl;
                
		for(int i = 0; i < meas_dim_; ++i) {
			F(i, i + meas_dim_) = dt;
		}

		   std::cout << "F_2 = " << F << std::endl;
		
		pbl::Vector x = G_.getMean();

                                   std::cout << "x = " << x << std::endl;
                
		for(int i = 0; i < meas_dim_; ++i) {
			x(i) += x(i + meas_dim_) * dt;
		}

		               std::cout << "x_2 = " << x << std::endl;
		
		G_.setMean(x);

		//		// set system noise
		double q = a_max_ * a_max_ / 4;
		double dt2 = dt * dt;
		double dt4 = dt2 * dt2;
                
                std::cout << "q, dt2, dt4 = " << q << ", " << dt2 << ", " << dt4 << std::endl;

       // Eigen::MatrixXd P = F * G_.getCovariance() * F.transpose();
                  pbl::Matrix P = F * G_.getCovariance() * F.t();
                    std::cout << "P = " << P << std::endl;
		for(int i = 0; i < meas_dim_; ++i) {
			P(i, i) += dt4 / 4 * q;						// cov pos
			P(i, i + meas_dim_) += dt4 / 4 * q;         // cov pos~vel
			P(i + meas_dim_, i + meas_dim_) += dt2 * q; // cov vel
		}
  std::cout << "P_2 = " << P << std::endl;
		G_.setCovariance(P);

		G_small_->setMean(H_ * G_.getMean());
		G_small_->setCovariance(H_ * G_.getCovariance() * H_.t());
                
                std::cout << "G_ = " << G_.toString() << std::endl;
                std::cout << "G_small_ = " << G_small_->toString() << std::endl;
                


//std::cout << "Limits = " << std::numeric_limits<float>::denorm_min() << std::endl;
                
                s = this->toString();
                std::cout << "KF propagate end: " << s << std::endl;
	}
}

void KalmanFilter::update(std::shared_ptr<const pbl::Gaussian> z) {
         std::string s = this->toString();
        
        std::cout << "KF update start: " << s << std::endl;
	//const Eigen::MatrixXd& x = G_.getMean();
	//const Eigen::MatrixXd& P = G_.getCovariance();
        const pbl::Vector& x = G_.getMean();
        const pbl::Matrix& P = G_.getCovariance();

	// determine innovation
	//Eigen::MatrixXd y = z.getMean() - H_ * x;
        pbl::Vector y = z->getMean() - H_ * x;

	// determine innovation covariance
	//Eigen::MatrixXd S = H_ * P * H_.transpose() + z->getCovariance();
        pbl::Matrix S = H_ * P * H_.t() + z->getCovariance();

	// calculate optimal Kalman gain
	//Eigen::MatrixXd K = P * H_.transpose() * S.inverse();
        pbl::Matrix K = P * H_.t() * inv(S);

	// update state
	G_.setMean(x + K * y);

	// update state covariance
        //Eigen::MatrixXd Identity;
        //Identity = Eigen::MatrixXd::Identity(state_dim_, state_dim_);
        //G_.setCovariance((Identity - K * H_) * P);
        G_.setCovariance((arma::eye(state_dim_, state_dim_) - K * H_) * P);

	G_small_->setMean(H_ * G_.getMean());
	G_small_->setCovariance(H_ * G_.getCovariance() * H_.t());
        s = this->toString();
        
        std::cout << "KF update end: " << s << std::endl;
}

double KalmanFilter::getLikelihood(const pbl::Gaussian& z) const {
	return z.getDensity(*G_small_);
}

std::shared_ptr<const pbl::Gaussian> KalmanFilter::getGaussian() const {
	return G_small_;
}

/*std::shared_ptr<const pbl::Gaussian> KalmanFilter::getGaussian() const {
        return G_small_;
}*/

const pbl::Vector& KalmanFilter::getState() const {
	return G_.getMean();
}

const pbl::Matrix& KalmanFilter::getStateCovariance() const {
	return G_.getCovariance();
}

void KalmanFilter::setMaxAcceleration(double a_max) {
	a_max_ = a_max;
}

std::string KalmanFilter::toString() const {
    std::stringstream s;
    
    s << "meas_dim_ = " << meas_dim_ << 
    " state_dim_ = "  << state_dim_ << 
    " G_ = " << G_.toString() << 
    " G_small_ = " << G_small_->toString() << 
    " p G_small_ = " << G_small_ <<
    " H_ = " << H_ << 
    " a_max_ = "  << a_max_;

    return s.str();
}
